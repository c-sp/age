//
// Copyright 2019 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

export enum AgeGbButton {
    GB_BUTTON_RIGHT = 0x01,
    GB_BUTTON_LEFT = 0x02,
    GB_BUTTON_UP = 0x04,
    GB_BUTTON_DOWN = 0x08,
    GB_BUTTON_A = 0x10,
    GB_BUTTON_B = 0x20,
    GB_BUTTON_SELECT = 0x40,
    GB_BUTTON_START = 0x80,
}


export class AgeGbKeyMap {

    private _keyMap: { [key: string]: AgeGbButton } = {
        "ArrowRight": AgeGbButton.GB_BUTTON_RIGHT,
        "ArrowLeft": AgeGbButton.GB_BUTTON_LEFT,
        "ArrowUp": AgeGbButton.GB_BUTTON_UP,
        "ArrowDown": AgeGbButton.GB_BUTTON_DOWN,
        "a": AgeGbButton.GB_BUTTON_A,
        "s": AgeGbButton.GB_BUTTON_B,
        "Enter": AgeGbButton.GB_BUTTON_SELECT,
        " ": AgeGbButton.GB_BUTTON_START,
    };

    getButtonForKey(key: string): AgeGbButton | undefined {
        return this._keyMap[key];
    }
}
