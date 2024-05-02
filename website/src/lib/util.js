export function formatTime(time, runType) {
    if (runType === 0) {
        return (time / 1000).toFixed(2).replace('.', ',') + "sek";
    } else {
        const minutes = Math.floor(time / 60000);
        const seconds = ((time % 60000) / 1000);
        return minutes + ":" + (seconds < 10 ? '0' : '') + seconds.toFixed(0) + "min";
    }
}