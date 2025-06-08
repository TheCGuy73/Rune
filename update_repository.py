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
    # Use the custom message if provided, otherwise the default one
    if user_commit_msg:
        commit_message = f"{user_commit_msg} [VERSION: {version}-{origin}/{branch}, COMMIT: {commit_format}]"
    else:
        commit_message = f"Update repository [VERSION: {version}-{origin}/{branch}, COMMIT: {commit_format}]"

    docs_dir = os.path.join(repo_path, "docs")
    cache_path = os.path.join(docs_dir, "cache.txt")
    history_path = os.path.join(docs_dir, "COMMIT_HISTORY.md")
    os.makedirs(docs_dir, exist_ok=True)

    with open(cache_path, "w", encoding="utf-8") as cache_file:
        cache_file.write(commit_message + "\n")

    # Configure remote
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

    # Update commit history BEFORE the commit
    if os.path.exists(cache_path):
        with open(cache_path, "r", encoding="utf-8") as cache_file:
            cache_content = cache_file.read()
        with open(history_path, "a", encoding="utf-8") as history_file:
            history_file.write(cache_content)
        os.remove(cache_path)

    run("git add .")
    commit_result = run(f'git commit -m "{commit_message}"')
    if commit_result != 0:
        print("No changes to commit.")
    else:
        run(f"git push {origin} {branch}")
    print(f"Current update: ({commit_message})")

def main():
    print("=== Git Repository Update ===")
    repo = input("Repository path (leave empty to use current folder): ").strip()
    if not repo:
        repo = os.getcwd()  # Use current folder as default
    origin = input("Remote name (e.g. origin): ").strip()
    branch = input("Branch name: ").strip()
    version = input("Version: ").strip()
    origin_url = input("Remote URL (leave empty for default): ").strip()
    user_commit_msg = input("Commit message (leave empty for default): ").strip()
    if not origin_url:
        origin_url = f"https://github.com/youruser/{os.path.basename(repo)}.git"
    if not user_commit_msg:
        user_commit_msg = None
    update_repository(version, origin, branch, origin_url, repo, user_commit_msg)

if __name__ == "__main__":
    main()