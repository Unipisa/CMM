
release:
	(VER=`/bin/ls -og curr | sed -e s/".* "// -e s/\\\./-/`; \
	tar zchf distrib/cmm-$${VER}.tgz cmm; \
	cd distrib; \
	/bin/rm cmm.tgz; \
	ln -s cmm-$${VER}.tgz cmm.tgz)
