# New Plan  25-05-2026
## planning to make the engine scalable.
## instead of only using hardcoded packages, use an additional AI model that classifies the packages
## hardcoded logic will be used for critical package checking, making sure some important packages are not flagged as removable
## AI model will return a list of packages that it classifies as Removable or Required.
## the packages that are flagged Removable will be send to hardcoded logic for final inspection.
## the packages that are not critical / not blocked by hardcoded logic will be selected
## finally, the selected packages are displayed in ui in removable categories like bolatware, analytics ect accorfing to their class.