import os
import datetime
def update_repository(origin: str, branch: str) -> None:
    CURRENT_DATE = datetime.datetime.now().strftime("%Y%m%d")
    CURRENT_TIME = datetime.datetime.now().strftime("%I%M%S")
    COMMIT_FORMAT = f"{CURRENT_DATE}:{CURRENT_TIME}"
    commit_message = str(input("Enter commit message: "))
    commit_message_formatted = commit_message + f" [{COMMIT_FORMAT}]"
    
    # Check if remote exists, if not, add it
    remote_check = os.system(f"git remote get-url {origin}")
    if remote_check != 0:
        os.system(f"git remote add {origin} <REMOTE_URL_HERE>")
    else:
        os.system(f"git remote set-url {origin} <REMOTE_URL_HERE>")
    os.system(f"git checkout {branch}")
    os.system("git pull")
    os.system("git add .")
    os.system(f'git commit -m "{commit_message_formatted}"')
    os.system(f"git push {origin} {branch}")

origin = input("Enter Origin: ")
branch = input("Enter Branch: ")
update_repository(origin, branch)