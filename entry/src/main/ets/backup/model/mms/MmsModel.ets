/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import fs from '@ohos.file.fs';

import DbConstants from '../../database/DbConstants';
import FileUtils from '../../../common/utils/FileUtils';
import IModel from '../IModel';
import LogUtils from '../../../common/utils/LogUtils';
import { dBHelper } from '../../database/DbHelper';
import common from '@ohos.app.ability.common';
import BackupExtensionContext from '@ohos.file.BackupExtensionContext';
import contextConstant from '@ohos.app.ability.contextConstant';
import hilog from '@ohos.hilog';
import SharedPreferencesUtils from '../../../common/utils/SharedPreferencesUtils';
import osAccount from '@ohos.account.osAccount';
import { newDbHelper } from '../../copyData/NewDbHelper';
import BackupRestoreProgress from '../../copyData/BackupRestoreProcess';
import {newDbHelperClone} from '../../database/NewDbHelper';
import BackupRestoreResult from '../../copyData/BackupRestoreResult';
import { BundleVersion } from '@kit.CoreFileKit';

const TAG = 'MmsModel';

export default class MmsModel implements IModel {
  /**
   * Start migrate database file in upgrade1
   */
  async migrateDbFile(context: BackupExtensionContext, progress: BackupRestoreProgress,
      bundleVersion: BundleVersion): Promise<string> {
    const newPath: string = context.databaseDir;
    let result: string = '';
    if (context.area !== contextConstant.AreaMode.EL2) {
      context.area = contextConstant.AreaMode.EL2;
    }
    const dbResPath = '/' + context.backupDir + 'restore/com.andr.providers.telephony/ce/databases/mmssms.db';
    const maapDbResPath = '/' + context.backupDir + 'restore/com.andr' + 'oid.providers.mms/ce/databases/maap.db';
    try {
      const bol = fs.accessSync(dbResPath);
      LogUtils.i(TAG, `onRestore accessSync old Db: ${bol}`);

      const bol1 = fs.accessSync(newPath);
      LogUtils.i(TAG, `onRestore accessSync new: ${bol1}`);

      const bol2 = fs.accessSync(maapDbResPath);
      LogUtils.i(TAG, `onRestore accessSync maapDbResPath Db: ${bol2}`);

      if (bol && bol1) {
        let dbDestPath = newPath + '/rdb/';
        if (fs.accessSync(dbDestPath)) {
          fs.rmdirSync(dbDestPath);
        }
        if (!fs.accessSync(dbDestPath)) {
          fs.mkdirSync(dbDestPath);
        }
        let maapDb = dbDestPath + 'maap.db'
        dbDestPath = dbDestPath + DbConstants.OLD_DATABASE_NAME;
        LogUtils.i(TAG, 'onRestore result: success');
        let ret = FileUtils.copyFile(dbResPath, dbDestPath);
        let ret1 = FileUtils.copyFile(maapDbResPath, maapDb);
        if (ret) {
          newDbHelper.deleteAllData();
          result = await this.startMigrateData(context, progress, bundleVersion);
        } else {
          LogUtils.i(TAG, 'copyFile failed');
          result = dBHelper.getExceptionResult();
        }
      }
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `db copy code:${e.code} msg:${e.message}`);
      LogUtils.i(TAG, `onRestore accessSync: false ${JSON.stringify(e)}`);
      result = dBHelper.getExceptionResult();
    }
    return result;
  }

  /**
   * Start migrate data after move db file in upgrade
   */
  async startMigrateData(context: common.Context, progress: BackupRestoreProgress,
      bundleVersion: BundleVersion): Promise<string> {
    LogUtils.i(TAG, 'startMigrateData');

    let accountManager: osAccount.AccountManager = osAccount.getAccountManager();
    let localId = -1;
    try {
      localId = await accountManager.getForegroundOsAccountLocalId();
      LogUtils.i(TAG, 'current localId: ' + localId);
    } catch (e) {
      LogUtils.e(TAG, 'getForegroundOsAccountLocalId exception: ' + JSON.stringify(e));
    }
    dBHelper.setVersion(bundleVersion.name);
    await dBHelper.queryUnreadCount(context, progress);
    await dBHelper.queryTotalMsgCount(context, localId, progress);
    await this.buildAddressTable(context, progress);
    progress.updateProcessCount(progress.getAddressMigrateProgress());
    let threadsCount = await dBHelper.queryThreadsCount(context, localId);
    let threadsPage = Math.ceil(threadsCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'threadsPage:' + threadsPage);
    for (let curPage = 1; curPage <= threadsPage; curPage++) {
      await dBHelper.queryThreadsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), localId, progress);
    }
    progress.updateProcessCount(progress.getThreadMigrateProgress());
    let rcsThreadsCount = await dBHelper.queryRcsThreadsCount(context, localId);
    let rcsThreadsPage = Math.ceil(rcsThreadsCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'rcsThreadsPage:' + rcsThreadsPage);
    for (let curPage = 1; curPage <= rcsThreadsPage; curPage++) {
      await dBHelper.queryRcsThreadsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), localId, progress);
    }
    progress.updateProcessCount(progress.getRcsThreadMigrateProgress());
    let smsCount = await dBHelper.querySmsCount(context, localId);
    let smsPage = Math.ceil(smsCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'smsPage:' + smsPage);
    for (let curPage = 1; curPage <= smsPage; curPage++) {
      await dBHelper.querySmsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), localId, progress);
    }
    progress.updateProcessCount(progress.getSmsMigrateProgress());
    let pduCount = await dBHelper.queryPduCount(context, localId);
    let pduPage = Math.ceil(pduCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'pduPage:' + pduPage);
    for (let curPage = 1; curPage <= pduPage; curPage++) {
      await dBHelper.queryPduInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), localId, progress);
    }
    progress.updateProcessCount(progress.getPduMigrateProgress());
    let partCount = await dBHelper.queryPartCount(context);
    let partPage = Math.ceil(partCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'partPage:' + partPage);
    for (let curPage = 1; curPage <= partPage; curPage++) {
      await dBHelper.queryPartInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getPartMigrateProgress());
    await dBHelper.updateRecordTime(context);
    await dBHelper.updateSmsMmsInfoByContent(context);
    await dBHelper.updateSessionDraft(context);

    let chatCount = await dBHelper.queryChatCount(context, localId);
    let chatPage = Math.ceil(chatCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'chatPage:' + chatPage);
    for (let curPage = 1; curPage <= chatPage; curPage++) {
      await dBHelper.queryChatInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), localId, progress);
    }
    progress.updateProcessCount(progress.getRcsChatMigrateProgress());
    let groupCount = await dBHelper.queryGroupCount(context);
    let groupPage = Math.ceil(groupCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'groupPage:' + groupPage);
    for (let curPage = 1; curPage <= groupPage; curPage++) {
      await dBHelper.queryRcsGroupInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getRcsGroupMigrateProgress());
    
    let maapCount = await dBHelper.queryMaapCount(context);
    let maapPage = Math.ceil(maapCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'maapPage:' + maapPage);
    for (let curPage = 1; curPage <= maapPage; curPage++) {
      await dBHelper.queryMaapInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getMaapMigrateProgress());

    let favSmsCount = await dBHelper.queryFavSmsCount(context);
    let favSmsPage = Math.ceil(favSmsCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'favSmsPage: ' + favSmsPage);
    for (let curPage = 1; curPage <= favSmsPage; curPage++) {
      await dBHelper.queryFavSmsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getFavSmsMigrateProgress());

    let favMmsCount = await dBHelper.queryFavMmsCount(context);
    let favMmsPage = Math.ceil(favMmsCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'favMmsPage: ' + favMmsPage);
    for (let curPage = 1; curPage <= favMmsPage; curPage++) {
      await dBHelper.queryFavMmsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    dBHelper.checkAndReportFavSmsMigrate();
    progress.updateProcessCount(progress.getFavMmsMigrateProgress());

    // make sure follow processing finished before next process update coming to avoid process check timeout
    await newDbHelper.deleteInvalidSession(context, progress);
    await newDbHelper.deleteInvalidSmsMmsInfo(context, progress);
    await newDbHelper.deleteInvalidRcsInfo(context, progress);
    progress.updateProcessCount(progress.getInvalidMigrateProgress());
    await dBHelper.updateSessionWithUnSupportItem(context, progress);
    progress.updateProcessCount(progress.getUnSupportItemMigrateProgress());
    await newDbHelper.recountAndUpdateUnread(context, progress);
    progress.updateProcessCount(progress.getTotalCount());
    return dBHelper.getUpgradeResult(context, localId);
  }

  private async buildAddressTable(context: common.Context, progress: BackupRestoreProgress) {
    let addressCount = await dBHelper.queryAddressCount(context);
    let addressPage = Math.ceil(addressCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'AddressPage:' + addressPage);
    for (let curPage = 1; curPage <= addressPage; curPage++) {
      await dBHelper.queryAddressInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
  }
}