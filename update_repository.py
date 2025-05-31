import os
import datetime

def update_repository(version: str, origin: str, branch: str) -> None:
    CURRENT_DATE = datetime.datetime.now().strftime("%Y%m%d")
    CURRENT_TIME = datetime.datetime.now().strftime("%I%M%S")
    COMMIT_FORMAT = f"{CURRENT_DATE}:{CURRENT_TIME}"
    VERSION = version
    BRANCH = branch
    ORIGIN = origin
    commit_message = str(input("Enter commit message: "))
    commit_message_formatted = commit_message + f" [VERSION: {VERSION}-{ORIGIN}/{BRANCH}, BUILD: {COMMIT_FORMAT}]"
    remote_url = input("Enter remote URL: ")

    # Percorsi nella cartella docs
    cache_path = os.path.join("docs", "cache.txt")
    history_path = os.path.join("docs", "COMMIT_HISTORY.md")

    # Assicurati che la cartella docs esista
    os.makedirs("docs", exist_ok=True)

    # Write the commit message to docs/cache.txt
    with open(cache_path, "w", encoding="utf-8") as cache_file:
        cache_file.write(commit_message_formatted + "\n")
    
    # Check if remote exists, if not, add it
    remote_check = os.system(f"git remote get-url {origin}")
    if remote_check != 0:
        os.system(f"git remote add {origin} {remote_url}")
    else:
        os.system(f"git remote set-url {origin} {remote_url}")
    os.system(f"git checkout {branch}")
    os.system("git pull")
    os.system("git add .")
    os.system(f'git commit -m "{commit_message_formatted}"')
    os.system(f"git push {origin} {branch}")
    print(f"Current update: ({commit_message_formatted}) ")

    # Append the commit message from docs/cache.txt to docs/COMMIT_HISTORY.md
    if os.path.exists(cache_path):
        with open(cache_path, "r", encoding="utf-8") as cache_file:
            cache_content = cache_file.read()
        with open(history_path, "a", encoding="utf-8") as history_file:
            history_file.write(cache_content)
        # Remove docs/cache.txt after transferring its content
        os.remove(cache_path)

origin = input("Enter Origin: ")
branch = input("Enter Branch: ")
version = input("Enter Version: ")

update_repository(version, origin, branch)