
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/* 最大経路長 */
	static constexpr size_t kMaxPathLength = 512;

	/*指定階層のファイル・フォルダ名一覧取得*/
	static bool CreateFilaNameList(const std::wstring& wstrFolderPath, const wchar_t* pwzFileNamePattern, std::vector<std::wstring>& wstrNames)
	{
		std::wstring wstrPath = wstrFolderPath;
		if (pwzFileNamePattern != nullptr)
		{
			if (wcschr(pwzFileNamePattern, L'*') == nullptr)
			{
				wstrPath += L'*';
			}
			wstrPath += pwzFileNamePattern;
		}
		else
		{
			wstrPath += L'*';
		}

		WIN32_FIND_DATAW sFindData;

		HANDLE hFind = ::FindFirstFileW(wstrPath.c_str(), &sFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (pwzFileNamePattern != nullptr)
			{
				do
				{
					/*ファイル一覧*/
					if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}
			else
			{
				do
				{
					/*フォルダ一覧*/
					if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
						{
							wstrNames.push_back(sFindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}

			::FindClose(hFind);
		}
		return wstrNames.size() > 0;
	}

	static bool MakeDirectory(const std::wstring& wstrPath)
	{
		BOOL iRet = ::CreateDirectoryW(wstrPath.c_str(), nullptr);
		if (iRet == 0)
		{
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
			{
				return true;
			}
		}
		else
		{
			return true;
		}

		return false;
	}
}

/*指定階層のファイル・フォルダ一覧作成*/
bool win_filesystem::CreateFilePathList(const wchar_t* folderPath, const wchar_t* fileSpec, std::vector<std::wstring>& paths, bool toAddParent)
{
	if (folderPath == nullptr || folderPath[0] == L'\0')return false;

	std::wstring wstrParent = folderPath;
	if (wstrParent.back() != L'\\')
	{
		wstrParent += L"\\";
	}
	std::vector<std::wstring> wstrNames;

	if (fileSpec != nullptr)
	{
		const auto SplitSpecs = [](const wchar_t* pwzFileSpec, std::vector<std::wstring>& specs)
			-> void
			{
				std::wstring wstrTemp;
				for (const wchar_t* p = pwzFileSpec; *p != L'\0' && p != nullptr; ++p)
				{
					if (*p == L';')
					{
						if (!wstrTemp.empty())
						{
							specs.push_back(wstrTemp);
							wstrTemp.clear();
						}
						continue;
					}

					wstrTemp.push_back(*p);
				}

				if (!wstrTemp.empty())
				{
					specs.push_back(wstrTemp);
				}
			};
		std::vector<std::wstring> specs;
		SplitSpecs(fileSpec, specs);

		for (const auto& spec : specs)
		{
			CreateFilaNameList(wstrParent, spec.c_str(), wstrNames);
		}
	}
	else
	{
		CreateFilaNameList(wstrParent, fileSpec, wstrNames);
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < wstrNames.size(); ++j)
		{
			if (::StrCmpLogicalW(wstrNames[nIndex].c_str(), wstrNames[j].c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames[i], wstrNames[nIndex]);
	}

	if (paths.empty())
	{
		paths = std::move(wstrNames);
		if (toAddParent)
		{
			for (std::wstring& path : paths)
			{
				path = wstrParent + path;
			}
		}
	}
	else
	{
		for (const std::wstring& wstr : wstrNames)
		{
			if (toAddParent)paths.emplace_back(wstrParent + wstr);
			else paths.push_back(wstr);
		}
	}

	return paths.size() > 0;
}
/*指定経路と同階層のファイル・フォルダ一覧作成・相対位置取得*/
bool win_filesystem::GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex)
{
	std::wstring wstrParent;

	size_t nPos = wstrPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrPath.substr(0, nPos);
	}

	win_filesystem::CreateFilePathList(wstrParent.c_str(), pwzFileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), wstrPath);
	if (iter != paths.cend())
	{
		*nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.cend();
}

std::wstring_view win_filesystem::GetCurrentProcessPath()
{
	static wchar_t s_basePath[kMaxPathLength]{};
	static size_t s_basePathLength = 0;
	if (s_basePath[0] == '\0')
	{
		static constexpr size_t basePathSize = sizeof(s_basePath) / sizeof(wchar_t);
		DWORD length = ::GetModuleFileNameW(nullptr, s_basePath, basePathSize);
		wchar_t* pEnd = s_basePath + length;
		for (; pEnd != s_basePath; --pEnd)
		{
			if (*pEnd == L'\\' || *pEnd == L'/')break;
		}

		wchar_t* pFileName = pEnd + 1;
		size_t fileNameLength = s_basePath + length - pFileName;
		memset(pFileName, '\0', fileNameLength);

		s_basePathLength = pFileName - s_basePath;
	}

	return std::wstring_view(s_basePath, s_basePathLength);
}

bool win_filesystem::CreateDirectoryStatic(std::wstring_view directoryName, wchar_t* dst, size_t& nWritten, size_t dstSize, std::wstring_view basePath)
{
	if (basePath.empty())
	{
		basePath = GetCurrentProcessPath();
	}

	if (dstSize < basePath.size())return false;

	wmemcpy(dst, basePath.data(), basePath.size());
	nWritten = basePath.size();

	size_t nRead = 0;
	if (dst[nWritten] != L'\\' && dst[nWritten] != L'/')
	{
		dst[nWritten++] = L'\\';
		dst[nWritten] = L'\0';

	}
	if (directoryName[0] == L'\\' || directoryName[0] == L'/')++nRead;

	for (;;)
	{
		size_t nPos = directoryName.find_first_of(L"\\/", nRead);
		if (nPos == std::wstring_view::npos)
		{
			const wchar_t* pRead = directoryName.data() + nRead;
			size_t nLength = directoryName.size() - nRead;
			if (dstSize < nWritten + nLength + 1)return false;

			wmemcpy(dst + nWritten, pRead, nLength);
			nWritten += nLength;
			dst[nWritten++] = L'\\';
			dst[nWritten] = L'\0';

			::CreateDirectoryW(dst, nullptr);

			break;
		}

		const wchar_t* pRead = &directoryName[nRead];
		size_t nLength = nPos - nRead;
		if (dstSize < nWritten + nLength + 1)return false;

		wmemcpy(dst + nWritten, pRead, nLength);
		nWritten += nLength;
		dst[nWritten++] = L'\\';

		::CreateDirectoryW(dst, nullptr);

		nRead = nPos + 1;
	}

	return true;
}
/*文字列としてファイル読み込み*/
std::string win_filesystem::LoadFileAsString(const wchar_t* filePath)
{
	std::string fileData;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD ulSize = INVALID_FILE_SIZE;
	DWORD ulRead = 0;
	BOOL iRet = FALSE;

	hFile = ::CreateFileW(filePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)goto end;

	ulSize = ::GetFileSize(hFile, nullptr);
	if (ulSize == INVALID_FILE_SIZE)goto end;

	fileData.resize(ulSize);
	iRet = ::ReadFile(hFile, &fileData[0], ulSize, &ulRead, nullptr);
	/* To suppress warning C28193 */
	if (iRet == FALSE)goto end;

end:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
	}

	return fileData;
}

bool win_filesystem::SaveStringToFile(const wchar_t* filePath, const void* pData, unsigned long dataLength, bool toOverWrite)
{
	if (filePath != nullptr)
	{
		HANDLE hFile = ::CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, toOverWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			DWORD nWritten = 0;
			BOOL iRet = ::WriteFile(hFile, pData, dataLength, &nWritten, nullptr);
			::CloseHandle(hFile);

			return iRet == TRUE;
		}
	}
	return false;
}

bool win_filesystem::DoesFileExist(const wchar_t* filePath)
{
	return ::PathFileExistsW(filePath) == TRUE;
}

bool win_filesystem::RenameFile(const wchar_t* filePathOld, const wchar_t* filePathNew)
{
	return ::MoveFileW(filePathOld, filePathNew) != 0;
}
