import os
import datetime
import subprocess

def run(cmd):
    result = subprocess.run(cmd, shell=True)
    return result.returncode

def update_repository(version, origin, branch, origin_url, repo_path, user_commit_msg=None):
    os.chdir(repo_path)
    current_date = datetime.datetime.now().strftime("%Y%m%d")
    current_time = datetime.datetime.now().strftime("%H%M%S")
    commit_format = f"{current_date}:{current_time}"
    # Usa il messaggio personalizzato se fornito, altrimenti quello di default
    if user_commit_msg:
        commit_message = user_commit_msg
    else:
        commit_message = f"Update repository [VERSION: {version}-{origin}/{branch}, COMMIT: {commit_format}]"

    docs_dir = os.path.join(repo_path, "docs")
    cache_path = os.path.join(docs_dir, "cache.txt")
    history_path = os.path.join(docs_dir, "COMMIT_HISTORY.md")
    os.makedirs(docs_dir, exist_ok=True)

    with open(cache_path, "w", encoding="utf-8") as cache_file:
        cache_file.write(commit_message + "\n")

    # Configura remote
    if run(f"git remote get-url {origin}") != 0:
        run(f"git remote add {origin} {origin_url}")
    else:
        run(f"git remote set-url {origin} {origin_url}")

    # Checkout branch
    if run(f"git rev-parse --verify {branch}") != 0:
        run(f"git checkout -b {branch}")
    else:
        run(f"git checkout {branch}")

    run("git pull")
    run("git add .")
    commit_result = run(f'git commit -m "{commit_message}"')
    if commit_result != 0:
        print("No changes to commit.")
    else:
        run(f"git push {origin} {branch}")
    print(f"Current update: ({commit_message})")

    # Aggiorna la cronologia dei commit
    if os.path.exists(cache_path):
        with open(cache_path, "r", encoding="utf-8") as cache_file:
            cache_content = cache_file.read()
        with open(history_path, "a", encoding="utf-8") as history_file:
            history_file.write(cache_content)
        os.remove(cache_path)

def main():
    print("=== Aggiornamento repository Git ===")
    repo = input("Percorso della repository (lascia vuoto per usare la cartella corrente): ").strip()
    if not repo:
        repo = os.getcwd()  # Usa la cartella corrente come default
    origin = input("Nome remote (es. origin): ").strip()
    branch = input("Nome branch: ").strip()
    version = input("Versione: ").strip()
    origin_url = input("URL remote (lascia vuoto per default): ").strip()
    user_commit_msg = input("Messaggio di commit (lascia vuoto per default): ").strip()
    if not origin_url:
        origin_url = f"https://github.com/youruser/{os.path.basename(repo)}.git"
    if not user_commit_msg:
        user_commit_msg = None
    update_repository(version, origin, branch, origin_url, repo, user_commit_msg)

if __name__ == "__main__":
    main()