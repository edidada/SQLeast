#include "rm/filehandle.h"
#include "sm/dbmanager.h"
#include "sm/systemmanager.h"

namespace sqleast {
    namespace sm {

        DBManager::DBManager(const char *dbName):
//                relCatalog_(rm::RecordManager::openFile(SystemManager::appendRelCatalogExt(dbName).c_str())),
//                attrCatalog_(rm::RecordManager::openFile(SystemManager::appendAttrCatalogExt(dbName).c_str()))
                  relCatalog_(rm::RecordManager::openFile(REL_CATALOG)),
                  attrCatalog_(rm::RecordManager::openFile(ATTR_CATALOG))
        {
        }

        DBManager::~DBManager() {
        }

        void DBManager::createTable(const char *relName, int attrNum, AttrInfo *attrs) {
            DataAttrInfo dataAttrInfo;
            memset(&dataAttrInfo, 0, sizeof(dataAttrInfo));
            int offset = 0;
            Record r2(FLAG_SIZE + sizeof(DataAttrInfo));
            for (int i = 0; i < attrNum; i++, attrs++) {
                strncpy(dataAttrInfo.relName, relName, MAX_NAME_LENGTH);
                strncpy(dataAttrInfo.attrName, (*attrs).attrName, MAX_NAME_LENGTH);
                dataAttrInfo.attrLength = (*attrs).attrLength;
                dataAttrInfo.attrType = (*attrs).attrType;
                dataAttrInfo.nullable = (*attrs).nullable;
                dataAttrInfo.indexNo = i;
                dataAttrInfo.offset = offset;
                dataAttrInfo.nullBitOffset = i / 8;
                dataAttrInfo.nullBitMask = 1 << (i % 8);

                memcpy(r2.getData(), &dataAttrInfo, sizeof(DataAttrInfo));
                attrCatalog_.insertRec(r2);

                offset += (*attrs).attrLength;
            }
            RelInfo relInfo;
            memset(&relInfo, 0, sizeof(relInfo));
            strncpy(relInfo.relName, relName, MAX_NAME_LENGTH);
            relInfo.tupleLength = offset;
            relInfo.bitmapSize = attrNum / 8 + (attrNum % 8 > 0);
            relInfo.attrCount = attrNum;
            relInfo.indexCount = 0;
            Record r(FLAG_SIZE + sizeof(RelInfo));
            memcpy(r.getData(), &relInfo, sizeof(RelInfo));
            relCatalog_.insertRec(r);

            rm::RecordManager::createFile(
                    SystemManager::appendDBExt(relInfo.relName).c_str(),
                    (int)FLAG_SIZE + relInfo.tupleLength + relInfo.bitmapSize,
                    false);
        }
    }
}