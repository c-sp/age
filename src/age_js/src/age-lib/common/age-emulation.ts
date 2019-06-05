//
// Copyright 2018 Christoph Sprenger
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

import {IEmGbModule} from "./em-modules";


// TODO lib
export class AgeEmulationPackage {

    constructor(readonly emGbModule: IEmGbModule,
                readonly romFileContents: ArrayBuffer) {
    }
}


// TODO app & lib
export interface IAgeEmulationRuntimeInfo {
    romName: string;
    emulatedSeconds?: number;
    emulationSpeed?: number;
    emulationMaxSpeed?: number;
}

// TODO lib
export function compareRuntimeInfo(x?: IAgeEmulationRuntimeInfo, y?: IAgeEmulationRuntimeInfo): boolean {
    return !!x && !!y
        && (x.romName === y.romName)
        && (x.emulatedSeconds === y.emulatedSeconds)
        && (x.emulationSpeed === y.emulationSpeed)
        && (x.emulationMaxSpeed === y.emulationMaxSpeed)
        || (!x && !y);
}
