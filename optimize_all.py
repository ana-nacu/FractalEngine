#!/usr/bin/env python3
import argparse, subprocess, pathlib, multiprocessing as mp, os, time, csv

HERE      = pathlib.Path(__file__).resolve().parent
CAT_RAW   = HERE / "lsysGrammar" / "Catalog"
CAT_OPT   = HERE / "lsysGrammar" / "Catalog_opt"
MESH_OPT  = HERE / "cmake-build-debug" / "licenta-cgal" / "mesh_opt"
def optimize_one(args):
    in_path, out_path, ratio = args
    # Dacă fișierul optimizat există și e mai nou decât sursa → skip
    if out_path.exists() and out_path.stat().st_mtime > in_path.stat().st_mtime:
        return f"[skip] {in_path.name}"

    t0 = time.time()
    cmd = [str(MESH_OPT), str(in_path), str(out_path), "--ratio", str(ratio)]
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"[error] {in_path.name} failed: {e}")
        return f"[error] {in_path.name}"
    dt = time.time() - t0
    CSV = HERE / "results" / "optimize_times.csv"
    CSV.parent.mkdir(exist_ok=True)
    with open(CSV, "a", newline="") as f:
        w = csv.writer(f)
        w.writerow([in_path.name, in_path.stat().st_size,
                    dt, out_path.stat().st_size])
    return f"[ok  ] {in_path.name}  ({dt:.1f}s)"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ratio", type=float, default=0.15,
                    help="Edge-ratio păstrat (0.15 = ~15 %)")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count()//2 or 1,
                    help="Procese paralele")
    args = ap.parse_args()

    CAT_OPT.mkdir(exist_ok=True)

    tasks = []
    for obj in CAT_RAW.glob("*.obj"):
        out = CAT_OPT / obj.name
        tasks.append((obj, out, args.ratio))

    with mp.Pool(args.jobs) as pool:
        for res in pool.imap_unordered(optimize_one, tasks):
            print(res)

if __name__ == "__main__":
    main()