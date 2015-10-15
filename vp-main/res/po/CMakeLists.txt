# for i18n

SET(POFILES ar.po az.po bg.po ca.po cs.po da.po de.po el_GR.po en_PH.po en.po en_US.po es_ES.po es_US.po et.po eu.po fi.po fr_CA.po fr.po ga.po gl.po
hi.po hr.po hu.po hy.po is.po it_IT.po ja_JP.po ka.po kk.po ko_KR.po lt.po lv.po mk.po nb.po nl.po pl.po pt_BR.po pt_PT.po ro.po
ru_RU.po sk.po sl.po sr.po sv.po tr_TR.po uk.po uz.po zh_CN.po zh_HK.po zh_TW.po)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})

	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)

	SET(vp_moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${vp_moFile}
			COMMAND ${MSGFMT} -o ${vp_moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	INSTALL(FILES ${vp_moFile}
			DESTINATION ${VP_LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(vp_moFiles ${vp_moFiles} ${vp_moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${vp_moFiles}")
ADD_CUSTOM_TARGET(vp_po ALL DEPENDS ${vp_moFiles})
