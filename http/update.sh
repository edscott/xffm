#!/bin/sh
rsync -av ./ --exclude='distfiles/*' xffm.sf.net:/home/groups/x/xf/xffm/htdocs
#rsync -av ./ mocha.foo-projects.org:/var/www/xffm.xfce.org
#rsync -av ./ espresso.foo-projects.org:/var/www/xffm.foo-projects.org
rsync -av ./ doppio.foo-projects.org:/var/www/xffm.foo-projects.org
