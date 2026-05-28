# New Plan  25-05-2026 
## planning to make the engine scalable.
## Add Ollama or other local AI models 
## How it will work :
## User will select packages that he is unsure about deleting, opts to ask AI.
## the app checks for installation of Ollama (some lightweight AI model)
## if yes then :
### pass the package to local AI; the AI decides if package is safe to remove or not by checking the web as well as a hardcoded logic which prevents critical packages like telephony, ims, fingerprint etc from deletion. after processing, it gives a brief description of what the package does or is it safe to remove and why. thus the user can now decide if he wants to remove it or not.
## else :
### prompt the user to either install or continue without one, where he will not have AI assisted package searching on web. If he chooses to install, the the app installs the AI model automatically, runs it. if user skips installation, them the feature for searching package on web via AI never appers to him until he changes his mind.
## the AI will only give its own decision in form of brief description which it has generated from web search + hardcoded rules (like ignoring ims, telephony, etc), the AI never forces user to delete or never automatically delete any package, instead, just gives its opinion which the user may or may not consider.