include( ../../common.pro )

TARGET		= dotnet$$KDEBUG

SOURCES = \
dotnet.cpp

system( tqmoc dotnet.h -o tqmoc/dotnet.moc )


