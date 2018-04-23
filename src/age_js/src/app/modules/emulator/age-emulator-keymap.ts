export enum AgeGbButton {
    GB_BUTTON_RIGHT = 0x01,
    GB_BUTTON_LEFT = 0x02,
    GB_BUTTON_UP = 0x04,
    GB_BUTTON_DOWN = 0x08,
    GB_BUTTON_A = 0x10,
    GB_BUTTON_B = 0x20,
    GB_BUTTON_SELECT = 0x40,
    GB_BUTTON_START = 0x80
}


export class AgeGbKeyMap {

    private _keyMap: { [key: string]: AgeGbButton } = {
        'ArrowRight': AgeGbButton.GB_BUTTON_RIGHT,
        'ArrowLeft': AgeGbButton.GB_BUTTON_LEFT,
        'ArrowUp': AgeGbButton.GB_BUTTON_UP,
        'ArrowDown': AgeGbButton.GB_BUTTON_DOWN,
        'a': AgeGbButton.GB_BUTTON_A,
        's': AgeGbButton.GB_BUTTON_B,
        'Enter': AgeGbButton.GB_BUTTON_SELECT,
        ' ': AgeGbButton.GB_BUTTON_START
    };

    getButtonForKey(key: string): AgeGbButton | undefined {
        return this._keyMap[key];
    }
}
