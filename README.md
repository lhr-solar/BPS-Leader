# Battery Protection System Leader
All firmware for the Battery Protection System Leader board.

## Setup
Development is done on a Linux environment to build, flash, and debug firmware

Follow this [guide](https://cloud.wikis.utexas.edu/wiki/spaces/LHRSOLAR/pages/28287146/Setting+up+an+Embedded+Development+Environment) to setup an Embedded Development Environment. We primarily use the Windows Subsystem for Linxu (make sure you are using WSL2 and not WSL1).  
After cloning the repo, enter the Embedded-Sharepoint directory an run `./install.sh` to install all needed dependencies. 


### Cloning the Repo
You must clone this repo using SSH. If you do not have an SSH key with Github setup then follow this [guide](https://docs.github.com/en/github/authenticating-to-github/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent). Note that you must follow the steps while in WSL.

Once SSH is setup and you can this to clone this repository. We append `` --recursive`` to the end to also clone the Embedded-Sharepoint reposistory. 
```
git clone --recursive
```
If you already cloned the repo but don't have the submodules, enter the following commands:

```
git submodule init
git submodule update --remote
```

## Building
Run `make help` for more information on usage.  
When calling any of the following commands, make sure you are in the top level of the repo.

Call `make` to build the release version of the codebase. By default the BPS builds for the STM32F413RHT microcontroller, but you can change that by changing the ` PROJECT_TARGET` field to be the mcu you'd like to compile for

Call `make clean` if the build fails for any reason other than syntax related.

### How to build a test
To build a new test, you need to use the following command:
`make TEST=x` where x is the name of the test file without the .c suffix.

## Flashing
Call `make flash` to flash the most recently built BPS code.  
If using WSL, you must use use usbipd to connect your port to WSL.

There is also a guide on how to flash with the Nucleos [here](https://cloud.wikis.utexas.edu/wiki/spaces/LHRSOLAR/pages/28286498/Flashing+and+debugging+code+on+an+MCU)


## Rules
Commit frequently into your own branches. Create a Pull Request whenever you are ready to add you working code to the master branch. Make sure that your code compiles without any errors or warnings before you open a pull request. At least one approver must approve your pull request before it can be merged. The reviewers will make sure everything is up to par with the coding standards.

### Code Reviews
You can review a pull request even if you are not an approver. In addition to helping us catch bugs, code reviews are a good opportunity to learn more about the BPS codebase. Some examples on what you can include in a code review are:
- Ask why the author wrote a certain part of code in a way they did
- Point out potential bugs in the code
- Point out that part of the code is not well documented or hard to read

You can either approve, comment, or request changes at the end of your pull request. These are the scenarios in which you would use each option:
- **Approve:** You are signing off that this pull request is ready to merge. You and the pull request author "own" this part of the code now.
- **Request Changes:** You have found problems with the code that must be addressed before it can be merged. Note that this will block the pull request from merging until you re-review the code.
- **Comment:** Do this if you do not want to explicitly approve or request changes.