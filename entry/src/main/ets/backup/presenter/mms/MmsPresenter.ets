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
import IPresenter from '../IPresenter';
import BackupExtensionContext from '@ohos.file.BackupExtensionContext';
import MmsModel from '../../model/mms/MmsModel';
import BackupRestoreProgress from './../../copyData/BackupRestoreProcess';
import { BundleVersion } from '@kit.CoreFileKit';

export default class MmsPresenter implements IPresenter {
  private static instance: MmsPresenter;

  private static sMmsModel: MmsModel;

  public static getInstance(): MmsPresenter {
    if (!MmsPresenter.instance) {
      MmsPresenter.instance = new MmsPresenter();
    }
    return MmsPresenter.instance;
  }

  init(): void {
    MmsPresenter.sMmsModel = new MmsModel();
  }

  async migrateDbFile(context: BackupExtensionContext, progress: BackupRestoreProgress,
    bundleVersion: BundleVersion): Promise<string> {
    return await MmsPresenter.sMmsModel.migrateDbFile(context, progress, bundleVersion);
  }
}