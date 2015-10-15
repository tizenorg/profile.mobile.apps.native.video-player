#!/bin/bash

#snapshot="SLPRelease_20121204.002"
#snapshot="SLP_20130208.002"

snapshot=$1

if [ $# -ne 1 ]
then
	echo ""
	echo "load_pkg : Get debug package from web"
	echo ""
	echo "Usage load_pkg [Snapshot version]" 
	echo ""
	exit 1;
fi

declare -i release

case $snapshot in
	RelRedwoodCISOPEN*)
		echo "CIS Open"
		url="http://165.213.148.149:8008/snapshots/rel/redwood/cis-open/"
		release=0
		;;
	RelRedwoodEUROPEN*)
		echo "Europe Open"
		url="http://165.213.148.149:8008/snapshots/rel/redwood/eur-open/"
		release=0
		;;
	SLPRelease*)
		echo "Release Mode"
		url="http://165.213.148.151/snapshots/release/"
		release=1
		;;
	DailyRedwoodEUROPEN*)
		echo "Daily Binary"
		url="http://165.213.148.149:8008/snapshots/daily/redwood/eur-open/"
		release=0
		;;

	DailyRedwoodLTEEURORA*)
		echo "Daily Binary Orange LTE"
		url="http://165.213.148.151/snapshots/daily/redwoodlte/eur-ora/"
		release=0
		;;

	RelRedwoodLTEEURORA*)
		echo "Release LTE orange Binary"
		url="http://165.213.148.151/snapshots/rel/redwoodlte/eur-ora/"
		release=0
		;;

	DailyRedwoodCISOPEN*)
		echo "Daily Redwood CIS Open"
		url="http://165.213.148.151/snapshots/daily/redwood/cis-open/"
		release=0
		;;

	DailyRedwoodEURORA*)
		echo "Daily Redwood Eur Ora"
		url="http://165.213.148.151/snapshots/daily/redwood/eur-ora/"
		release=0
		;;

	SLP_*)
		echo "Normal mode"
		url="http://165.213.148.149:8008/snapshots/standard/"
		release=0
		;;
	*) 
		echo "Uknown snapshot name: " $snapshot
		exit 1;
		;;
esac

mkdir -p $snapshot; cd $snapshot

echo "Addr=$url$snapshot/repos"

pkg="eglibc app-core-efl evas ecore elementary edje eina mmfw capi-media-player capi-media-sound-manager capi-media-image-util capi-content-media-content libmm-common libmm-sound libmm-session libmm-utility libmm-fileinfo libmm-transcode libmm-player libmedia-thumbnail"

url_repo=$url$snapshot"/repos"

if [ -e tmp ]; then
	rm tmp
fi

if [ -e pkg_list ]; then
	rm pkg_list
fi

#libstd++ is in http://165.213.148.149:8008/snapshots/standard/SLP_20130208.002/repos/slp-base/armv7l/packages/armv7l/libstdc++-4.5.3-11.3.slp.armv7l.rpm  
if [ $release -eq 1 ]
then
	url_base=$url_repo"/slp-base/armv7l/debug/"
	url_release=$url_repo"/slp-release/armv7l/debug/"
	url_package=$url_repo"/slp-base/armv7/packages/armv7l"

#	echo DBG=$url_base
#	echo DBG=$url_release

	lynx -dump $url_base >> tmp
	lynx -dump $url_release >> tmp


	for pkgname in $pkg
	do
		if [ "$pkgname" == "libstdc++" ]
		then 
			echo $url_package
			lynx -dump $url_package | grep 'http' | egrep "/libstdc\+\+-[0-9]" | awk '{print $2}' >> pkg_list
		else
			grep 'http' tmp | grep "/$pkgname" | awk '{print $2}' >> pkg_list
		fi
	done

elif [ $release -eq 0 ]
then
	url_base=$url_repo"/slp-release/armv7l/debug/"
#	url_standard=$url_repo"/slp-standard/armv7l/debug/"
#	url_package=$url_repo"/slp-base/armv7l/packages/armv7l"

#	echo DBG=$url_base
#	echo DBG=$url_standard
	lynx -dump $url_base >> tmp
#	lynx -dump $url_standard >> tmp

	for pkgname in $pkg
	do
		if [ "$pkgname" == "libstdc++" ]
		then 
			echo $url_package
			lynx -dump $url_package | grep 'http' | egrep "/libstdc\+\+-[0-9]" | awk '{print $2}' >> pkg_list
		else
			grep 'http' tmp | grep "/$pkgname" | awk '{print $2}' >> pkg_list
		fi
	done
fi

for x in $(cat pkg_list)
do
	echo "Get: '${x##*/}"
	wget -q -nd -N $x
done

rm tmp
rm pkg_list

