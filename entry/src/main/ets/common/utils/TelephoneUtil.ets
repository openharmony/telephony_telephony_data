/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import LogUtils from './LogUtils';
import call from '@ohos.telephony.call';
import { BusinessError } from '@ohos.base';
import telephonySim from '@ohos.telephony.sim';
import { StringUtil } from './StringUtil';

const TAG = 'TelephoneUtil';

const infoMegTelephone: string[] = [
// 国家类服务号码
  '9', '95', '100', '101', '106', '108', '111', '116', '118', '123', '400', '800', '1212', '000000', '114', '0898',
  '12580', '12520', '12583', '0452',
  // 国家类区行政号码开头
  '\\d{2,4}12123$', '\\d{2,4}12329$', '^02[0-9]', '010', '^05[34]', '^07[56]',
];

class TelephoneUtil {
  public judgeIsInfoMsg(telephone: string): boolean {
    if (StringUtil.isEmpty(telephone)) {
      return false;
    }
    let regexSymbol = new RegExp('\\,')
    if (regexSymbol.test(telephone)) {
      return false;
    }
    let regex = new RegExp('^(0086|\\+?86|\\+)?(' + infoMegTelephone.join('|') + ')');
    return regex.test(telephone);
  }

  public async formatPhoneNumberToE164(oriNumber: string, slotId: number): Promise<string> {
    if (StringUtil.isEmpty(oriNumber)) {
      return '';
    }
    let countryIso: string = '';
    let localNumber: string = '';
    await telephonySim.getISOCountryCodeForSim(slotId).then(data => {
      countryIso = data;
    }).catch((err: BusinessError) => {
      LogUtils.e(TAG, 'getISOCountryCodeForSim by subId: ' + slotId + ' iso error: ' + JSON.stringify(err));
    });
    await call.formatPhoneNumberToE164(oriNumber, countryIso).then((data) => {
      if (data !== null) {
        localNumber = data;
      }
    }).catch((error: BusinessError) => {
      LogUtils.e(TAG, 'formatPhoneNumberToE164 error: ' + JSON.stringify(error));
    });
    return localNumber;
  }
}

// Singleton
export default new TelephoneUtil();