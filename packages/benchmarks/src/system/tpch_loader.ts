import * as arrow from 'apache-arrow';
import * as arrow3 from 'apache-arrow-3';
import * as path from 'path';
import * as fs from 'fs';
import * as fsAsync from 'fs/promises';

export function getTPCHSQLiteDBPath(base: string, sf: number): string {
    const dirName = sf.toString().replace('.', '_');
    const filePath = path.resolve(base, 'data', 'tpch', dirName, 'sqlite.db');
    if (!fs.existsSync(filePath)) {
        throw new Error(`file does not exists: ${filePath}`);
    }
    return filePath;
}

export async function getTPCHSQLiteDB(base: string, sf: number): Promise<Buffer> {
    const dbPath = getTPCHSQLiteDBPath(base, sf);
    return await fsAsync.readFile(dbPath);
}

export function getTPCHCSVFilePath(base: string, sf: number, name: string): string {
    const dirName = sf.toString().replace('.', '_');
    const filePath = path.resolve(base, 'data', 'tpch', dirName, 'tbl', name);
    if (!fs.existsSync(filePath)) {
        throw new Error(`file does not exists: ${filePath}`);
    }
    return filePath;
}

export function getTPCHParquetFilePath(base: string, sf: number, name: string): string {
    const dirName = sf.toString().replace('.', '_');
    const filePath = path.resolve(base, 'data', 'tpch', dirName, 'parquet', name);
    if (!fs.existsSync(filePath)) {
        throw new Error(`file does not exists: ${filePath}`);
    }
    return filePath;
}

export function getTPCHArrowFilePath(base: string, sf: number, name: string): string {
    const dirName = sf.toString().replace('.', '_');
    const filePath = path.resolve(base, 'data', 'tpch', dirName, 'arrow', name);
    if (!fs.existsSync(filePath)) {
        throw new Error(`file does not exists: ${filePath}`);
    }
    return filePath;
}

export async function getTPCHArrowTable(base: string, sf: number, name: string): Promise<arrow.Table> {
    const filePath = await getTPCHArrowFilePath(base, sf, name);
    const buffer = await fsAsync.readFile(filePath);
    return arrow.tableFromIPC(buffer);
}

export async function getTPCHArrow3Table(base: string, sf: number, name: string): Promise<arrow3.Table> {
    const filePath = await getTPCHArrowFilePath(base, sf, name);
    const buffer = fsAsync.readFile(filePath);
    const reader = await arrow3.AsyncRecordBatchFileReader.from(buffer);
    const batches: arrow3.RecordBatch[] = [...reader];
    return new arrow3.Table(batches[0].schema, batches);
}

export async function getTPCHQuery(base: string, name: string): Promise<string> {
    const scriptDir = path.resolve(base, 'packages', 'benchmarks', 'scripts', 'tpch');
    return await fsAsync.readFile(path.resolve(scriptDir, name), 'utf-8');
}
