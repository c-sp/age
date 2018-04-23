import {Directive} from '@angular/core';


@Directive({
    selector: '[ageLoaderError]'
})
export class AgeLoaderErrorDirective {
}


@Directive({
    selector: '[ageLoaderSuccess]'
})
export class AgeLoaderSuccessDirective {
}


@Directive({
    selector: '[ageLoaderWorking]'
})
export class AgeLoaderWorkingDirective {
}
