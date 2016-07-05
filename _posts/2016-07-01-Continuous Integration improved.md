---
layout: post
title: Continuous Integration improvements
---
{% include JB/setup %}

Release process has been drastically automated, thanks to the powerful tools from github, travis and appveyor. Making a release is now (almost) as easy as tagging the right branch in the github repo: it automatically creates the releases and upload the binaries. So OpenJPEG  will eventually have a release cycle that’s worth its name: every 3 months at max, I’ll tag the repo to trigger a new release.

And, among the bunch of tests that are automatically done for each commit, the results of the API/ABI compliance check is now directly uploaded on the OpenJPEG website: http://www.openjpeg.org/abi-check/timeline/openjpeg/, so that we can follow the compliance from one commit to another.