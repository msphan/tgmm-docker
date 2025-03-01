#include "DivisionClassifierFactory.h"

#include <experimental/filesystem>
#include "TGMMsupport.h"
#include "CellDivisionLookupTable.h"

using namespace std;

static const char* kind_name_table[] = {
	"DivisionClassiferKind_None",
	"DivisionClassiferKind_BoostedElipticalHaarFeatures_AmatF2013",
	"DivisionClassiferKind_LookupTable_2018"
};

/**
 * \brief A Mock CellDivisionClassifier that actually just does nothing.
 */
class CellDivisionDoNothing: public IDivisionClassifier {
public:

	CellDivisionDoNothing()=default;

	virtual ~CellDivisionDoNothing()=default;

	int setClassifierModel(std::string filename) { return 0; }
	int classifyCellDivisionTemporalWindow(lineageHyperTree& lht,int frame,std::vector<mylib::Array*>& imgVec,int devCUDA) final { return 0; }
	size_t getNumCellDivisions()  { return 0; }
	float getThrCDWT()  { return 0; }
	void setThrCDWT(float p)  {};

	std::ostream& print(std::ostream& os) {
		return os<<"DivisionClassifer(None)";
	}
};

/**
 * \brief This is the internal state used by the factory.  
 *        It's hidden in here just to isolate it from the outside world.
 */
struct DivisionClassifierFactory::State {
	DivisionClassiferKind kind;
	shared_ptr<IDivisionClassifier> instance; // this pointer is "owned" by the factory

	explicit State(
		std::string classifier_file_root_path,
		configOptionsTrackingGaussianMixture::CellDivisionClassifier& config)
	: kind(config.method) 
	{
		switch(config.method) {
			case DivisionClassiferKind_BoostedElipticalHaarFeatures_AmatF2013:
			{
				const auto model=classifier_file_root_path+"/"+config.Amatf2013.classifierFileCDTW;
				config.Amatf2013.classifierFileCDTW=model;
				instance=make_shared<cellDivisionTemporalWindow_TGMMsupport>(classifier_file_root_path,config.Amatf2013);
			}
			break;
			case DivisionClassiferKind_LookupTable_2018:
			{
				instance=make_shared<CellDivisionLookupTable>(config.LUT);							
			}
			break;
			case DivisionClassiferKind_None:
			{
				instance=make_shared<CellDivisionDoNothing>();
			}
			break;
			default:
				throw runtime_error("DivisionClassifierFactory: invalid kind.");
		}
	}

	~State() = default;
};


DivisionClassifierFactory::DivisionClassifierFactory(
	std::string root_path,
	configOptionsTrackingGaussianMixture::CellDivisionClassifier& config)
: state(nullptr)
{
	state=new State(root_path,config);
}

DivisionClassifierFactory::~DivisionClassifierFactory() {
	delete state;
}

const std::shared_ptr<IDivisionClassifier>& DivisionClassifierFactory::get() const {
	return state->instance;
}

std::ostream& operator<<(std::ostream& os , const DivisionClassifierFactory& f) {	
	return f.state->instance->print(os);
}