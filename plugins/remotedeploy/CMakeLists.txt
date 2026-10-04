include(BuildVeyonPlugin)

build_veyon_plugin(remotedeploy
	NAME RemoteDeploy
	SOURCES
	RemoteDeployFeaturePlugin.cpp
	RemoteDeployFeaturePlugin.h
	RemoteDeployDialog.cpp
	RemoteDeployDialog.h
	remotedeploy.qrc
	)
