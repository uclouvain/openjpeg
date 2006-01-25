# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "21:00:00 EDT")

# Dart server to submit results (used by client)
SET (DROP_METHOD "http")
SET (DROP_SITE "public.kitware.com")
SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
SET (TRIGGER_SITE 
    "http://${DROP_SITE}/cgi-bin/Submit-Public-TestingResults.cgi")

# Not used right now, since on public:
# Project Home Page
SET (PROJECT_URL "http://www.openjpeg.org")

# Dart server configuration
#SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/openjpeg-rollup-dashboard.sh")
SET (CVS_WEB_URL "http://euterpe.tele.ucl.ac.be/cgi-bin/viewcvs.cgi/")
SET (CVS_WEB_CVSROOT "OpenJPEG")

SET (USE_DOXYGEN "On")
SET (DOXYGEN_URL "http://www.openjpeg.org/libdoc/index.html")
