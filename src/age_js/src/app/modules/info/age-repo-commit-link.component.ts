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

import {ChangeDetectionStrategy, Component} from '@angular/core';
import {VERSION_INFO} from '../../../environments/version';


@Component({
    selector: 'age-repo-commit-link',
    template: `
        <a href="https://gitlab.com/csprenger/AGE/tree/{{commitHash}}"
           target="_blank"
           rel="noopener nofollow noreferrer"
           title="Link to the git commit this version of AGE is based on"
           class="age-ui-clickable">

            <ng-content></ng-content>
            {{commitHash | slice:0:8}}

        </a>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeRepoCommitLinkComponent {

    readonly commitHash = VERSION_INFO.hash;
}
