include(BuildVeyonPlugin)

build_veyon_plugin(quizplatform
	NAME QuizPlatform
	SOURCES
	QuizPlatformFeaturePlugin.cpp
	QuizPlatformFeaturePlugin.h
	QuizPlatformDialog.cpp
	QuizPlatformDialog.h
	quizplatform.qrc
	)
