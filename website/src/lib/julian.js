export function dateToJulian(date) {
    const now = date ? new Date(date) : new Date();
    const time = now.getTime();
    return Math.floor((time / 86400000) - (now.getTimezoneOffset() / 1440) + 2440587.5);
}

export function julianToDate(julianDay) {
    const time = (julianDay - 2440587.5 + (new Date().getTimezoneOffset() / 1440)) * 86400000;
    return new Date(time);
}
