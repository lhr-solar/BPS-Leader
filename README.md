# Battery Protection System Leader
All firmware for the Battery Protection System Leader board

## Setup
Development is done on a Linux environment to build, flash, and debug firmware

Follow this [guide](https://cloud.wikis.utexas.edu/wiki/spaces/LHRSOLAR/pages/28287146/Setting+up+an+Embedded+Development+Environment) to setup an Embedded Development Environment. We primarily use the Windows Subsystem for Linxu (make sure you are using WSL2 and not WSL1).

### Cloning the Repo
You must clone this repo using SSH. If you do not have an SSH key with Github setup then follow this [guide](https://docs.github.com/en/github/authenticating-to-github/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent). Note that you must follow the steps while in WSL.

Once SSH is setup and you can this to clone this repository. We append `` --recursive`` to the end to also clone the Embedded-Sharepoint reposistory. 
```
git clone --recursive
```

## Building
