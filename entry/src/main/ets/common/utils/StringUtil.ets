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
import buffer from '@ohos.buffer';
import util from '@ohos.util';

/**
 * Handle string util
 */
export class StringUtil {
  /**
   * If string is empty
   *
   * @param str which to handle
   * @returns if string is empty
   */
  static isEmpty(str: string): boolean {
    return str === null || str.length === 0 || str === '';
  }

  /**
   * Get data type from mime type
   *
   * @param str which to handle
   * @returns  removed space str
   */
  static getType(mimeType: string | undefined): string {
    if (mimeType === undefined || StringUtil.isEmpty(mimeType)) {
      return '';
    }
    if (mimeType.indexOf('/') === -1) {
      return '';
    }
    let type = mimeType.split('/')[0];
    if (StringUtil.isEmpty(type)) {
      return '';
    }
    return type;
  }

  /**
   * Get tails of the path
   *
   * @param str path
   * @returns  tail
   */
  static getPathTail(path: string | undefined): string {
    if (path === undefined || StringUtil.isEmpty(path)) {
      return '';
    }
    if (path.indexOf('/') === -1) {
      return '';
    }
    let arr = path.split('/');
    let result = arr[arr.length - 1];
    if (StringUtil.isEmpty(result)) {
      return '';
    }
    return result;
  }

  /**
   * Remove str space
   *
   * @param str which to handle
   * @returns string removed space
   */
  static removeSpace(str: string): string {
    if (StringUtil.isEmpty(str)) {
      return '';
    }
    str = str.replace(/\\s/g, '');
    return str;
  }

  /**
   * Map to Record
   *
   * @param Map
   * @returns Record
   */
  static Map2Rec(map:Map<string, ESObject>): Record<string, ESObject> {
    let rec:Record<string, ESObject> = {};
    map.forEach((value:ESObject, key:string) => {
      if (value instanceof Map) {
        let vRec:Record<string, ESObject> = StringUtil.Map2Rec(value);
        value = vRec;
      }
      rec[key] = value;
    })
    return rec
  }

  /**
   * iso-8859-1 to utf-8, resolving garbled mms sub problems
   *
   * @param iso-8859-1 format string
   * @returns utf-8 format string
   */
  static codeConversion(rawStr: string): string {
    let buf = buffer.from(rawStr, 'latin1');
    let uintArrSub = new Uint8Array(buf.buffer);
    const decoder = util.TextDecoder.create('utf-8');
    let newStr = decoder.decodeWithStream(uintArrSub);
    return newStr;
  }

  /**
   * check str contains 'sip',case insensitive
   *
   * @param str String on which to perform the search
   * @returns a Boolean value that indicates whether or not a pattern exists in a searched string.
   */
  static isSip(str: string): boolean {
    const regex = /sip/i;
    return regex.test(str);
  }
}