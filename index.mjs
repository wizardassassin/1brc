import fs from "fs";
import readline from "readline";

const rl = readline.createInterface({
    input: fs.createReadStream(process.argv[2] || "./measurements.txt"),
    crlfDelay: Infinity,
});

/**
 * @type {Map<string, {min: number, max: number, mean: number, count: number}>}
 */
const data = new Map();

/**
 *
 * @param {string} line
 */
const parseLine = (line) => {
    const [station, temp] = line.split(";");
    const temp2 = Number(temp);
    let info = data.get(station);
    if (!info) {
        info = {
            min: Infinity,
            max: -Infinity,
            sum: 0,
            count: 0,
        };
        data.set(station, info);
    }
    info.min = Math.min(info.min, temp2);
    info.max = Math.max(info.max, temp2);
    info.sum += temp2;
    info.count++;
};

rl.on("line", (line) => {
    parseLine(line);
});

rl.once("close", () => {
    const arr = [...data];
    arr.sort((a, b) => a[0].localeCompare(b[0]));
    const arr2 = arr.map(
        ([station, info]) =>
            `${station}=${info.min.toFixed(1)}/${(
                info.sum / info.count
            ).toFixed(1)}/${info.max.toFixed(1)}`
    );
    console.log("{" + arr2.join(", ") + "}");
});
