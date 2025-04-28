
#include <unordered_map>

#include "karinchan.h"

#include "win_filesystem.h"
#include "win_text.h"
#include "text_utility.h"
#include "path_utility.h"

/* �����p */
namespace karinchan
{
	struct SCommandDatum
	{
		std::wstring commandToken;
		std::unordered_map<std::wstring, std::wstring> params;
	};

	/* �w�ߕ���� */
	static void ParseCommand(const std::wstring& line, SCommandDatum& commandDatum)
	{
		size_t nStart = line.find(L'[');
		size_t nEnd = line.rfind(L']');
		if (nStart == std::wstring::npos || nEnd == std::wstring::npos)return;

		++nStart;
		size_t nPos = line.find(L' ', nStart);
		if (nPos == std::wstring::npos)
		{
			/* �ϐ��w�薳�� */
			commandDatum.commandToken = line.substr(nStart, nEnd - nStart);
			return;
		}
		else
		{
			commandDatum.commandToken = line.substr(nStart, nPos - nStart);
			++nPos;
		}

		std::wstring paramName;
		std::wstring paramValue;

		for (size_t nRead = nPos; nRead < nEnd; ++nRead)
		{
			const wchar_t p = line[nRead];
			if (p == L' ')
			{
				paramValue = line.substr(nPos, nRead - nPos);
				commandDatum.params.insert({ paramName, paramValue });
				nPos = nRead + 1;

				paramValue.clear();
				paramName.clear();
			}
			else if (p == L'=')
			{
				paramName = line.substr(nPos, nRead - nPos);
				nPos = nRead + 1;
			}
		}

		if (!paramName.empty())
		{
			paramValue = line.substr(nPos, nEnd - nPos);
			commandDatum.params.insert({ paramName, paramValue });
		}
	}

} /* namespace karinchan */

/* ��{�ǂݎ�� */
bool karinchan::ReadScenario(const std::wstring& wstrFolderPath, std::vector<adv::TextDatum>& textData, std::vector<adv::ImageFileDatum>& imageFileData, std::vector<adv::SceneDatum>& sceneData)
{
	std::wstring wstrScenarioFilePath = wstrFolderPath + L"\\script.txt";

	std::wstring wstrFile = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrScenarioFilePath.c_str()));
	if (wstrFile.empty())return false;

	std::vector<std::wstring> lines;
	text_utility::TextToLines(wstrFile, lines);

	adv::TextDatum textDatumBuffer;
	adv::SceneDatum sceneDatumBuffer;

	for (const auto& line : lines)
	{
		if (line.size() < 2)continue;
		if (line[0] == L'/' && line[1] == L'/')continue;

		if (line[0] != L'[')
		{
			 /* �n�̕��E�䎌���e */
			size_t nPos = line.rfind(L"[p]");
			if (nPos != std::wstring::npos)
			{
				/* ��؂� */
				textDatumBuffer.wstrText += line.substr(0, nPos);

				textData.push_back(textDatumBuffer);
				textDatumBuffer = adv::TextDatum();

				sceneDatumBuffer.nTextIndex = textData.size() - 1;
				sceneData.push_back(sceneDatumBuffer);
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
				/* �n�̕��E�䎌�J�n */
				if (!commandDatum.params.empty())
				{
					auto iter = commandDatum.params.find(L"name");
					if (iter != commandDatum.params.cend())
					{
						textDatumBuffer.wstrName = iter->second;
						text_utility::ReplaceAll(textDatumBuffer.wstrName, L"#user#", L"�꓁");
					}

					iter = commandDatum.params.find(L"voice");
					if (iter != commandDatum.params.cend())
					{
						text_utility::ReplaceAll(iter->second, L"\"", L"");
						textDatumBuffer.wstrVoiceFilePath = wstrFolderPath + L"\\" + path_utility::ExtractFileName(iter->second) + L".m4a";
					}
				}
			}
			else if (commandDatum.commandToken == L"cgstart")
			{
				/* �Î~��w�� */
				const auto &iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.bAnimation = false;
					text_utility::ReplaceAll(iter->second, L"\"", L"");
					imageFileDatum.wstrFilePath = wstrFolderPath + L"\\" + path_utility::TruncateFilePath(iter->second);
					
					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;
				}
			}
			else if (commandDatum.commandToken == L"animestart")
			{
				/* �A�j���[�V�����w�� */
				auto iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.bAnimation = true;
					text_utility::ReplaceAll(iter->second, L"\"", L"");
					imageFileDatum.wstrFilePath = wstrFolderPath + L"\\" + path_utility::TruncateFilePath(iter->second);

					iter = commandDatum.params.find(L"anim");
					if (iter == commandDatum.params.cend())continue;
					unsigned long ulIndex = wcstoul(iter->second.c_str(), nullptr, 10);
					imageFileDatum.animationParams.usIndex = static_cast<unsigned short>(ulIndex);

					iter = commandDatum.params.find(L"loop");
					if (iter == commandDatum.params.cend())continue;
					imageFileDatum.animationParams.bLoop = iter->second == L"1";

					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;
				}
			}
		}
	}

	return true;
}
