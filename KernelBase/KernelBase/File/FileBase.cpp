#include "FileBase.h"

BOOLEAN FsCreateFile(UNICODE_STRING ustrFilePath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	//�����ļ�
	InitializeObjectAttributes(&objectAttributes, &ustrFilePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}
	//�رվ��
	ZwClose(hFile);
	return TRUE;
}


BOOLEAN FsCreateFolder(UNICODE_STRING ustrFolderPath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	//����Ŀ¼
	InitializeObjectAttributes(&objectAttributes, &ustrFolderPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE, FILE_DIRECTORY_FILE, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}
	ZwClose(hFile);
	return TRUE;
}


BOOLEAN FsDeleteFileOrFolder(UNICODE_STRING ustrFileName)
{
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };

	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	//ִ��ɾ������
	status = ZwDeleteFile(&objectAttributes);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}
	return TRUE;
}


ULONG64 FsGetFileSize(UNICODE_STRING ustrFileName)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	FILE_STANDARD_INFORMATION fsi = { 0 };

	//��ȡ�ļ����
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, 0, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		return 0;
	}
	//��ȡ�ļ��Ĵ�С
	status = ZwQueryInformationFile(hFile, &iosb, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		return 0;
	}

	ZwClose(hFile);
	return fsi.EndOfFile.QuadPart;
}


BOOLEAN FsRenameFileOrFolder(UNICODE_STRING ustrSrcFileName, UNICODE_STRING ustrDestFileName)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	PFILE_RENAME_INFORMATION pRenameInfo = NULL;
	ULONG ulLenth = (1024 + sizeof(FILE_RENAME_INFORMATION));

	//�����ڴ�
	pRenameInfo = (PFILE_RENAME_INFORMATION)ExAllocatePool(NonPagedPool, ulLenth);
	if (pRenameInfo == NULL)
	{
		return FALSE;
	}
	//������������Ϣ
	RtlZeroMemory(pRenameInfo, ulLenth);
	pRenameInfo->FileNameLength = ustrDestFileName.Length;
	wcsncpy(pRenameInfo->FileName, ustrDestFileName.Buffer, ustrDestFileName.Length);
	pRenameInfo->ReplaceIfExists = FALSE;
	pRenameInfo->RootDirectory = NULL;
	//����Դ�ļ���Ϣ����ȡ���
	InitializeObjectAttributes(&objectAttributes, &ustrSrcFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(
		&hFile,
		SYNCHRONIZE | DELETE,
		&objectAttributes,
		&iosb,
		NULL,
		0,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NO_INTERMEDIATE_BUFFERING,
		NULL,
		0
		);

	if (!NT_SUCCESS(status))
	{
		ExFreePool(pRenameInfo);
		return FALSE;
	}
	//���iosb
	RtlZeroMemory(&iosb, sizeof(iosb));
	//����ZwSetInfomationFile�������ļ���Ϣ
	status = ZwSetInformationFile(hFile, &iosb, pRenameInfo, ulLenth, FileRenameInformation);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		ExFreePool(pRenameInfo);
		return FALSE;
	}
	//�ͷ��ڴ�,�رվ��
	ExFreePool(pRenameInfo);
	ZwClose(hFile);
	return TRUE;
}


BOOLEAN FsQueryFileAndFolder(UNICODE_STRING ustrPath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	//��ȡ�ļ����
	InitializeObjectAttributes(&objectAttributes, &ustrPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(
		&hFile,
		FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_ANY_ACCESS,
		&objectAttributes,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
		NULL,
		0
		);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	//�����ļ�
	//ע������Ĵ�С,һ��Ҫ�����㹻���ڴ�,�������ExFreePool������
	ULONG ulLength = (2 * 4096 + sizeof(FILE_BOTH_DIR_INFORMATION)) * 0x2000;
	PFILE_BOTH_DIR_INFORMATION pDir = (PFILE_BOTH_DIR_INFORMATION)ExAllocatePool(PagedPool, ulLength);//���������˷�ҳ�ڴ�
	//����pDir���׵�ַ,�����ͷ��ڴ�ʹ��
	PFILE_BOTH_DIR_INFORMATION pBeginAddr = pDir;
	//��ȡ��Ϣ
	status = ZwQueryDirectoryFile(
		hFile, 
		NULL, 
		NULL, 
		NULL, 
		&iosb, 
		pDir, 
		ulLength,
		FileBothDirectoryInformation, 
		FALSE, 
		NULL, 
		FALSE
		);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pDir);
		ZwClose(hFile);
		return FALSE;
	}
	//����
	UNICODE_STRING ustrTemp;
	UNICODE_STRING ustrOneDot;
	UNICODE_STRING ustrTwoDot;
	RtlInitUnicodeString(&ustrOneDot, L".");
	RtlInitUnicodeString(&ustrTwoDot, L"..");
	WCHAR wzFileName[1024] = { 0 };
	while (true)
	{
		//�ж��Ƿ����ϼ�Ŀ¼���߱�Ŀ¼
		RtlZeroMemory(wzFileName, 1024);
		RtlCopyMemory(wzFileName, pDir->FileName, pDir->FileNameLength);
		RtlInitUnicodeString(&ustrTemp, wzFileName);
		if ((RtlCompareUnicodeString(&ustrTemp, &ustrOneDot, TRUE) != 0) && (RtlCompareUnicodeString(&ustrTemp, &ustrTwoDot, TRUE) != 0))
		{
			if (pDir->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//Ŀ¼(��¼����Ŀ¼)
			}
			else
			{
				//�ļ�(��¼�����ļ�)
			}
		}
		//�������
		if (pDir->NextEntryOffset == 0)
		{
			break;
		}
		//pDirָ��ĵ�ַ�ı���,��������ExFreePool(pDir)�����!!!����,���뱣���׵�ַ
		pDir = (PFILE_BOTH_DIR_INFORMATION)((PUCHAR)pDir + pDir->NextEntryOffset);
	}
	//�ͷ��ڴ�,�ر��ļ����
	ExFreePool(pBeginAddr);
	ZwClose(hFile);
	return TRUE;
}

BOOLEAN FsReadFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pReadData, PULONG pulReadDataSize)
{
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	//���ļ�
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(
		&hFile,
		GENERIC_READ,
		&objectAttributes,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}
	//��ȡ�ļ�����
	RtlZeroMemory(&iosb, sizeof(iosb));
	status = ZwReadFile(
		hFile, 
		NULL, 
		NULL, 
		NULL, 
		&iosb, 
		pReadData, 
		*pulReadDataSize, 
		&liOffset, 
		NULL
		);
	if (!NT_SUCCESS(status))
	{
		*pulReadDataSize = iosb.Information;
		ZwClose(hFile);
		return FALSE;
	}
	//��ȡʵ�ʶ�ȡ������
	*pulReadDataSize = iosb.Information;
	//�رվ��
	ZwClose(hFile);
	return TRUE;
}

BOOLEAN FsWriteFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pWriteData, PULONG pulWriteDataSize)
{
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	//���ļ�
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(
		&hFile,
		GENERIC_WRITE,
		&objectAttributes,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}
	//��ȡ�ļ�����
	RtlZeroMemory(&iosb, sizeof(iosb));
	status = ZwWriteFile(
		hFile,
		NULL,
		NULL,
		NULL,
		&iosb,
		pWriteData,
		*pulWriteDataSize,
		&liOffset,
		NULL
		);
	if (!NT_SUCCESS(status))
	{
		*pulWriteDataSize = iosb.Information;
		ZwClose(hFile);
		return FALSE;
	}
	//��ȡʵ��д�������
	*pulWriteDataSize = iosb.Information;
	//�رվ��
	ZwClose(hFile);

	return TRUE;
}

BOOLEAN FsCopyFile(UNICODE_STRING ustrSrcFile, UNICODE_STRING ustrDestFile)
{
	ULONG ulBufferSize = 4096 * 10;
	ULONG ulReadDataSize = ulBufferSize;
	LARGE_INTEGER liOffset = { 0 };
	PUCHAR pBuffer = (PUCHAR)ExAllocatePool(NonPagedPool, ulBufferSize);//ʹ�÷Ƿ�ҳ�ڴ�

	//һ�߶�ȡ,һ��д��,ʵ���ļ�����
	do 
	{
		//��ȡ�ļ�
		ulReadDataSize = ulBufferSize;
		FsReadFile(ustrSrcFile, liOffset, pBuffer, &ulReadDataSize);
		//����ȡ������Ϊ�յ�ʱ��,�������Ʋ���
		if (ulReadDataSize <= 0)
		{
			break;
		}
		//д���ļ�
		FsWriteFile(ustrDestFile, liOffset, pBuffer, &ulReadDataSize);
		//����ƫ��
		liOffset.QuadPart = liOffset.QuadPart + ulReadDataSize;
	} while (TRUE);
	//�ͷ��ڴ�
	ExFreePool(pBuffer);
	return TRUE;
}