#pragma once

#include <ntifs.h>

BOOLEAN FsCreateFile(UNICODE_STRING ustrFilePath);				//�����ļ�
BOOLEAN FsCreateFolder(UNICODE_STRING ustrFolderPath);			//�����ļ���
BOOLEAN FsDeleteFileOrFolder(UNICODE_STRING ustrFileName);		//ɾ���ļ����ļ���(������ļ��е����,�ļ��б���Ϊ��,����ɾ��ʧ��)
ULONG64 FsGetFileSize(UNICODE_STRING ustrFileName);				//��ȡָ���ļ���ʵ�ʴ�С(��λ�ֽ�)
BOOLEAN FsRenameFileOrFolder(UNICODE_STRING ustrSrcFileName, UNICODE_STRING ustrDestFileName); //�������ļ����ļ���
BOOLEAN FsQueryFileAndFolder(UNICODE_STRING ustrPath);			//����ָ�����ļ���
BOOLEAN FsReadFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pReadData, PULONG pulReadDataSize);//��ȡ�ļ�
BOOLEAN FsWriteFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pWriteData, PULONG pulWriteDataSize);//д���ļ�
BOOLEAN FsCopyFile(UNICODE_STRING ustrSrcFile, UNICODE_STRING ustrDestFile);//�����ļ�