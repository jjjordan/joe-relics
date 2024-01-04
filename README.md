# joe-relics

This repository holds a collection of older versions of JOE, found from
various sources:

* `collection/usenet`: USENET messages and shar files from which several
  versions were released until 1.0.5
* `collection/cvs`: The full CVS respository that was used from version
  2.9 through 3.7
* `collection/downloads`: Archives downloaded from several sources
  (archives and otherwise) found across the internet. These would have
  originally been hosted on FTP sites in the mid-nineties.
* `collection/joe-hist`: The contents of the joe-hist archive released
  on Sourceforge, several years ago.
* `versions`: Contents of each complete release found in the above
  locations.

The Mercurial repository is still readily-available on
[Sourceforge](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/)
and is not included here.

### gitport

Finally, the `gitport` directory contains source code and metadata necessary
to construct the git repository with full history and release tags. As this
is a one-shot deal, it is mostly a curiosity.

## Early JOE (< 2.9) analysis

Early JOE comprises 31 versions made in approximately 11 sprints (clusters
of releases made in quick succession).  Of these, almost half (15 versions)
survive, with representation here from every sprint.  Some releases from
this era were included in early Linux distributions, which accounted for
most of the editor's popularity:

* 1.0.0 in [SLS](https://en.wikipedia.org/wiki/Softlanding_Linux_System) 1.02
* 1.0.8 in Slackware 2.0.1, [MCC](https://en.wikipedia.org/wiki/MCC_Interim_Linux) 1.0, and some later [SLS](https://en.wikipedia.org/wiki/Softlanding_Linux_System) version
* 2.2 in [Slackware](https://en.wikipedia.org/wiki/Slackware) 2.1 through 3.4
* 2.3 in [RedHat](https://en.wikipedia.org/wiki/Red_Hat_Linux) 1.1 Mother's Day Release
* 2.8 in just about every distribution from 1995 and on

### Timeline

* **E** *1988-09-20*
* **0.x** series (7 versions in 2-3 parts)
	* First edition:
		* **J** *1991-08-21*
		* **0.1.0** *1991-09-03*
	* **0.1.1** *unknown*
	* Second edition:
		* **0.1.2** *1992-01-23*
		* **0.1.3** *unknown*
		* **0.1.4** *1992-01-30*
	* **0.1.5** *before 06-20, probably before 03-27*
* **1.0.x** series (14 versions in 4 parts)
	* 1992 series
		* **1.0.0** *1992-11-16*
			* [SLS](https://en.wikipedia.org/wiki/Softlanding_Linux_System) 1.02
		* **1.0.1** *1992-11-17*
		* **1.0.2** *unknown*
		* **1.0.3** *1992-11-18*
		* **1.0.4** *unknown*
		* **1.0.5** *1992-11-21*
		* **1.0.6** *1992-11-21*
		* **1.0.7** *1992-11-26*
		* **1.0.8** *1992-12-19*
			* [Slackware](https://en.wikipedia.org/wiki/Slackware) 2.0.1
			* [MCC](https://en.wikipedia.org/wiki/MCC_Interim_Linux) 1.0
			* [SLS](https://en.wikipedia.org/wiki/Softlanding_Linux_System)
	* 1994 series
		* **1.0.9** *1994-04-24*
		* **1.0.10** *1994-08-09*
		* **1.0.11** *1994-08-10*
		* **1.0.12** *1994-08-18*
			* [Slackware](https://en.wikipedia.org/wiki/Slackware) 2.3, possibly as early as 2.1
		* **1.0.13** *1994-08-24*
* **2.x** series (9 versions in 3 parts)
	* 1994 series: (3 versions in ~1 part)
		* **2.0** *probably 1994-09-22 or 1994-09-23*
		* **2.1** *1994-09-23*
		* **2.2** *1994-10-06*
			* [Slackware](https://en.wikipedia.org/wiki/Slackware) 2.3 through 3.4
	* Christmas update:
		* **2.3** 1994-12-24 [1 versions, 1 part]
			* [RedHat](https://en.wikipedia.org/wiki/Red_Hat_Linux) 1.1 (Mother's Day Release)
	* 1995 series: (5 versions in 1 part)
		* **2.4** *1995-01-07*
		* **2.5** *1995-01-08*
		* **2.6** *1995-01-10*
		* **2.7** *unknown*
		* **2.8** *1995-01-23* Final update
			* Everywhere

Some notable missing releases:
* 1.0.13: The final update of the 1.0.x series
* 2.0-2.1: The first posted changes post-refactor
