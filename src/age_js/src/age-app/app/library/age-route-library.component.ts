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

import {ChangeDetectionStrategy, Component, OnDestroy} from "@angular/core";
import {AgeLayoutService} from "../common/age-layout.service";


@Component({
    selector: "age-library",
    template: ``,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRouteLibraryComponent implements OnDestroy {

    constructor(private readonly _layoutService: AgeLayoutService) {
        this._layoutService.libraryRouteActive = true;
    }

    ngOnDestroy(): void {
        this._layoutService.libraryRouteActive = false;
    }
}
