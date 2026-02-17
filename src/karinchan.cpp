
#include <unordered_map>

#include "karinchan.h"

#include "win_filesystem.h"
#include "win_text.h"
#include "text_utility.h"
#include "path_utility.h"

/* 内部用 */
namespace karinchan
{
	struct SCommandDatum
	{
		std::wstring_view commandToken;
		std::unordered_map<std::wstring_view, std::wstring_view> params;
	};

	/* 指令文解析 */
	static void ParseCommand(std::wstring_view line, SCommandDatum& commandDatum)
	{
		size_t nStart = line.find(L'[');
		size_t nEnd = line.rfind(L']');
		if (nStart == std::wstring_view::npos || nEnd == std::wstring_view::npos)return;

		++nStart;
		size_t nPos = line.find(L' ', nStart);
		if (nPos == std::wstring_view::npos)
		{
			/* 変数指定無し */
			commandDatum.commandToken = std::wstring_view(&line[nStart], nEnd - nStart);
			return;
		}
		else
		{
			commandDatum.commandToken = std::wstring_view(&line[nStart], nPos - nStart);
			++nPos;
		}

		std::wstring_view paramName;
		std::wstring_view paramValue;

		for (size_t nRead = nPos; nRead < nEnd; ++nRead)
		{
			const wchar_t p = line[nRead];
			if (p == L' ')
			{
				paramValue = std::wstring_view(&line[nPos], nRead - nPos);
				commandDatum.params.insert({ paramName, paramValue });
				nPos = nRead + 1;

				paramName = {};
			}
			else if (p == L'=')
			{
				paramName = std::wstring_view(&line[nPos], nRead - nPos);
				nPos = nRead + 1;
			}
		}

		if (!paramName.empty())
		{
			paramValue = std::wstring_view(&line[nPos], nEnd - nPos);
			commandDatum.params.insert({ paramName, paramValue });
		}
	}

	static std::wstring_view UnQuote(std::wstring_view s)
	{
		if (s.size() > 2 && s.front() == L'"' && s.back() == L'"')
		{
			s.remove_prefix(1);
			s.remove_suffix(1);
		}

		return s;
	}

} /* namespace karinchan */

/* 台本読み取り */
bool karinchan::ReadScenario(
	const std::wstring& wstrFolderPath,
	std::vector<adv::TextDatum>& textData,
	std::vector<adv::ImageFileDatum>& imageFileData,
	std::vector<adv::SceneDatum>& sceneData,
	std::vector<adv::LabelDatum>& labelData)
{
	std::wstring wstrScenarioFilePath = wstrFolderPath + L"\\script.txt";

	std::wstring wstrFile = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrScenarioFilePath.c_str()));
	if (wstrFile.empty())return false;

	std::vector<std::wstring_view> lines;
	text_utility::TextToLines(wstrFile, lines);

	adv::TextDatum textDatumBuffer;
	adv::SceneDatum sceneDatumBuffer;
	/* A label to the scene where still or animation will be switched. */
	std::wstring labelBuffer;

	for (const auto& line : lines)
	{
		if (line.size() < 2)continue;
		if (line[0] == L'/' && line[1] == L'/')continue;

		if (line[0] != L'[')
		{
			 /* 地の文・台詞内容 */
			size_t nPos = line.rfind(L"[p]");
			if (nPos != std::wstring::npos)
			{
				/* 区切り */
				textDatumBuffer.wstrText.append(line.data(), nPos);

				textData.push_back(std::move(textDatumBuffer));

				sceneDatumBuffer.nTextIndex = textData.size() - 1;
				sceneData.push_back(sceneDatumBuffer);

				if (!labelBuffer.empty())
				{
					labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
					labelBuffer.clear();
				}

			}
			else
			{
				textDatumBuffer.wstrText += line;
			}
		}
		else
		{
			SCommandDatum commandDatum;
			ParseCommand(line, commandDatum);

			if (commandDatum.commandToken == L"textcmd")
			{
				/* 地の文・台詞開始 */
				if (!commandDatum.params.empty())
				{
					auto iter = commandDatum.params.find(L"name");
					if (iter != commandDatum.params.cend())
					{
						textDatumBuffer.wstrName = iter->second;
						text_utility::ReplaceAll(textDatumBuffer.wstrName, L"#user#", L"一刀");
					}

					iter = commandDatum.params.find(L"voice");
					if (iter != commandDatum.params.cend())
					{
						std::wstring_view filePath = UnQuote(iter->second);
#ifdef TO_FLATTEN_RESOURCE_PATH
						textDatumBuffer.wstrVoiceFilePath.assign(wstrFolderPath).append(L"\\").append(path_utility::ExtractFileNameWithoutExtension(filePath)).append(L".m4a");
#else
						textDatumBuffer.wstrVoiceFilePath.assign(wstrFolderPath).append(L"\\").append(path_utility::ReplaceFullExtension(filePath, std::wstring_view(L".m4a")));
#endif
					}
				}
			}
			else if (commandDatum.commandToken == L"cgstart")
			{
				/* 静止画指定 */
				const auto &iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					std::wstring_view filePath = UnQuote(iter->second);

					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.isAnimation = false;
					/* 一部台本に".jpg"指定あり */
#ifdef TO_FLATTEN_RESOURCE_PATH
					imageFileDatum.wstrFilePath.assign(wstrFolderPath).append(L"\\").append(path_utility::ExtractFileNameWithoutExtension(filePath)).append(L".png");
#else
					imageFileDatum.wstrFilePath.assign(wstrFolderPath).append(L"\\").append(path_utility::ReplaceFullExtension(filePath, std::wstring_view(L".png")));
#endif
				
					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;

					labelBuffer = path_utility::ExtractFileNameWithoutExtension(iter->second);
				}
			}
			else if (commandDatum.commandToken == L"animestart")
			{
				/* 動作指定 */
				auto iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					std::wstring_view filePath = UnQuote(iter->second);

					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.isAnimation = true;
#ifdef TO_FLATTEN_RESOURCE_PATH
					imageFileDatum.wstrFilePath.assign(wstrFolderPath).append(L"\\").append(path_utility::TruncateFilePath(filePath));
#else
					imageFileDatum.wstrFilePath.assign(wstrFolderPath).append(L"\\").append(filePath);
#endif
					const auto &animIter = commandDatum.params.find(L"anim");
					if (animIter == commandDatum.params.cend())continue;
					unsigned long ulIndex = wcstoul(animIter->second.data(), nullptr, 10);
					imageFileDatum.animationParams.usIndex = static_cast<unsigned short>(ulIndex);

					iter = commandDatum.params.find(L"loop");
					if (iter == commandDatum.params.cend())continue;
					imageFileDatum.animationParams.loop = iter->second == L"1";

					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;

					labelBuffer.assign(animIter->first).append(L"_").append(animIter->second);
				}
			}
		}
	}

	return true;
}
