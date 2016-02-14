# How to contribute

So you want to contribute to Colobot: Gold Edition? That's awesome! Before you start, read this page, it contains a lot of useful information on how to do so.

## General information
Before you start, read more about technical details of the project. They can be found in our [Game Design Document](http://compiled.colobot.info/jenkins/job/colobot-gold-gdd/lastSuccessfulBuild/artifact/Colobot_Gold_Edition-Game_Design_Document.pdf) ([live editor](http://krzysh.pl:3000/project/545e90d99e8bceed2284797e)).

You may also find some useful information on the our (outdated) [dev wiki](http://colobot.info/wiki/dev).

## Before you start coding
* If you want to fix a bug, please first check the related [issue on GitHub's bug tracker](https://github.com/colobot/colobot/issues). If there isn't one, make it.
* If you want to add a new feature or make any change to gameplay, please first discuss it either [on the forums](http://colobot.info/forum) or in the related issue on GitHub. When your issue gets *accepted* label, that means that your suggestion got accepted and is waiting for somebody to work on it. Always wait for your suggestion to be accepted before you start writing any code.
* Before you start, check *"Assignee"* field in the issue and read the comments to see if nobody else is working on the same issue. If somebody is assigned to it, but there was no activity for a long time, you can take it over. Also, please post a comment on the issue that you want to help us, so other people don't waste time working at that issue in the same time.

## Coding style
See the [related page on dev wiki](http://colobot.info/wiki/dev/Coding_rules) for general guidelines, or [this file](https://github.com/colobot/colobot-lint/blob/master/RULES.md) for detailed description.

See [colobot-lint repository](https://github.com/colobot/colobot-lint) for automated tool that checks the coding style.

If your pull request breaks the coding style, you will have to fix it before it gets merged.

## Submitting pull requests
After you finish working on your issue and want your code to be merged into the main repository, you should submit a **pull request**. Go to [this page](https://github.com/colobot/colobot/pulls) and select "New pull request". All pull request should ALWAYS be submitted to the *dev* branch. After your PR gets reviewed by our development team, it will be merged to *dev* branch, and on the next release - to the *master* branch.

If you need more help, see [GitHub's help page on Pull Requests](https://help.github.com/articles/using-pull-requests/).

## Need help?
Ask on our official Freenode channel (#colobot @ irc.freenode.net) or on our [forums](http://colobot.info/forum/). We're here to help :)
