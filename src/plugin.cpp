#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
    p->addModel(modelSimpleStereoEcho);
    p->addModel(modelSimpleOSCvarSaw);
    p->addModel(modelSoftclip);
    p->addModel(modelVarShapeOSC);
    p->addModel(modelMoggle);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
