---
layout: post
title: Project migrated from googlecode to github + new website
---
{% include JB/setup %}

Better late than never, OpenJPEG has finally entirely moved to github.

- the repo is now at: <https://github.com/uclouvain/openjpeg>
- test data is now in a separate repo at: <https://github.com/uclouvain/openjpeg-data>
- the issue tracker is now at: <https://github.com/uclouvain/openjpeg/issues>
- the wiki is there: <https://github.com/uclouvain/openjpeg/wiki>

I invite you to update your working copies to link to the new repo. 
I’ve disabled yesterday the issue tab on googlecode, and removed existing commit rights.

A few comments about the migration:

- source has been migrated with whole history of revisions
- thanks to some (human) support from github, we were able to keep original timestamps of the issues and comments.
- issue and comment authorship could *not* be migrated: everything is authored by « gcode-importer ». However, the original author is always mentioned and in many cases, the link has been made to the corresponding github account (if we knew the mapping between gcode and github). This means that original authors will *not* get notified in case of issue updates so I invite interested people to subscribe to issues if needed.
- if issues had an assignee (« owner » in googlecode), it has been kept.

What is still to be done

- Reformat wiki: for now, pages have simply been imported « as is » from gcode, so a refactoring is needed here.
- Clean up obsolete branches in the repo.
- Continuous integration: update CDash, setup travis (?), configure jenkins (we have now a dedicated server for this).

I saw there are already a few forks of the repo, feel free to submit your pull request if you think what you did in your fork could benefit all openjpeg users. For those not familiar with this workflow, I invite them to read the github doc ([here](https://help.github.com/articles/using-pull-requests/) for instance) as pull requests will now be the preferred workflow to suggest changes for the library.

Last but not least, OpenJPEG has also a new website that makes use of the powerful [Jekyll](http://jekyllrb.com/) engine made available by Github Pages.