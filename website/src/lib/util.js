export function formatTime(time, runType) {
    if (runType === 0) {
        return (time / 1000).toFixed(2).replace('.', ',') + "sek";
    } else {
        const minutes = Math.floor(time / 60000);
        const seconds = ((time % 60000) / 1000);
        return minutes + ":" + (seconds < 10 ? '0' : '') + seconds.toFixed(0) + "min";
    }
}

export function formatDate(date) {
    return date.toLocaleDateString('de-DE', {
        day: '2-digit',
        month: '2-digit',
        year: 'numeric'
    });
}

export function dateToJulian(date) {
    const now = date ? new Date(date) : new Date();
    const time = now.getTime();
    return Math.floor((time / 86400000) - (now.getTimezoneOffset() / 1440) + 2440587.5);
}

export function julianToDate(julianDay) {
    const time = (julianDay - 2440587.5 + (new Date().getTimezoneOffset() / 1440)) * 86400000;
    return new Date(time);
}
