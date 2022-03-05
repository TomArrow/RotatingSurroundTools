
#include <iostream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bw64/bw64.hpp>
#include "adm/export.h"
#include <adm/adm.hpp>
#include <adm/utilities/object_creation.hpp>
#include <adm/write.hpp>



typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec3_t vec3_t_axes[3];

// From JK2 source code
// GPL!
#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
vec_t VectorNormalize(vec3_t v) {
	float	length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if (length) {
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}
void VectorRotate(const vec3_t in, const vec3_t matrix[3], vec3_t out)
{
	out[0] = DotProduct(in, matrix[0]);
	out[1] = DotProduct(in, matrix[1]);
	out[2] = DotProduct(in, matrix[2]);
}
static void S_MixSpatialize(const vec3_t origin, const vec3_t listenOrigin, const vec3_t_axes listenAxis,  vec3_t admPosition = nullptr) {
	vec_t		dot;
	vec_t		dist;
	vec_t		lscale, rscale, scale;
	vec3_t		source_vec;
	vec3_t		vec;


	// calculate stereo seperation and distance attenuation
	VectorSubtract(origin, listenOrigin, source_vec);

	dist = VectorNormalize(source_vec);
	vec_t absoluteDist = dist;

	VectorRotate(source_vec, listenAxis, vec);

	dot = -vec[1];
	rscale = 0.5 * (1.0 + dot);
	lscale = 0.5 * (1.0 - dot);

	if (admPosition) { // Todo: Figure out if I have the correct signedness here or if I need to invert some.
		admPosition[0] = -vec[1] * absoluteDist; // left/right
		admPosition[1] = vec[0] * absoluteDist; // front/back
		admPosition[2] = vec[2] * absoluteDist; // up/down
	}

}

// Function from: https://stackoverflow.com/a/1120224
// https://creativecommons.org/licenses/by-sa/4.0/
// Martin York
std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
{
	std::vector<std::string>   result;
	std::string                line;
	std::getline(str, line);

	std::stringstream          lineStream(line);
	std::string                cell;

	while (std::getline(lineStream, cell, ','))
	{
		result.push_back(cell);
	}
	// This checks for a trailing comma with no data after it.
	if (!lineStream && cell.empty())
	{
		// If there was a trailing comma then add an empty element.
		result.push_back("");
	}
	return result;
}





const unsigned int BLOCK_SIZE = 4096;
inline long long S_FrameTimeToNanoSeconds(long& inTime,double fps) {
	double timeInSeconds = ((double)inTime) / (fps);
	return (long long)(0.5 + timeInSeconds * 1000000000.0);
}
typedef struct vector3 {
	vec3_t data;
};
typedef struct axes {
	vec3_t_axes data;
};
// Main function that does stuff
int main(int argc, char** argv) {

	std::cout << argc << "\n";
	if (argc < 5) {

		std::cout << "Need at least 4 arguments: Audio file, speaker position csv, cam orientation CSV, fps";
		return 1;
	}
	std::string audioFile(argv[1]);
	std::string speakerPositionCSV(argv[2]);
	std::string camOrientationCSV(argv[3]);
	float fps = atof(argv[4]);

	std::cout << audioFile << "\n" << speakerPositionCSV << "\n" << camOrientationCSV << "\n" << fps << "\n";


	std::vector<vector3> channelPositions;

	std::filebuf fb;
	if (fb.open(speakerPositionCSV, std::ios::in))
	{
		std::istream is(&fb);
		while (is) {

			std::vector<std::string> coordinates = getNextLineAndSplitIntoTokens(is);
			if (coordinates.size() == 3) {

				vector3 pos;
				pos.data[0] = atof(coordinates[0].c_str());
				pos.data[1] = atof(coordinates[1].c_str());
				pos.data[2] = atof(coordinates[2].c_str());
				channelPositions.push_back(pos);
			}
		}
		fb.close();
	}

	std::cout << "Found " << channelPositions.size() << " speaker positions.";


	// Read orientation data for all frames.
	std::vector<axes> orientations;

	if (fb.open(camOrientationCSV, std::ios::in))
	{
		std::istream is(&fb);
		if (is) {
			getNextLineAndSplitIntoTokens(is); // skip first line, it's just the colum names
		}
		while (is) {

			std::vector<std::string> coordinates = getNextLineAndSplitIntoTokens(is);
			if (coordinates.size() >=9 ) {

				axes ori;
				ori.data[0][0] = atof(coordinates[0].c_str());
				ori.data[0][1] = atof(coordinates[1].c_str());
				ori.data[0][2] = atof(coordinates[2].c_str());
				ori.data[1][0] = atof(coordinates[3].c_str());
				ori.data[1][1] = atof(coordinates[4].c_str());
				ori.data[1][2] = atof(coordinates[5].c_str());
				ori.data[2][0] = atof(coordinates[6].c_str());
				ori.data[2][1] = atof(coordinates[7].c_str());
				ori.data[2][2] = atof(coordinates[8].c_str());
				orientations.push_back(ori);
			}
		}
		fb.close();
	}

	std::cout << "Found " << orientations.size() << " camera orientations.";



	std::vector<std::vector<vector3>> dynamicChannelPositions;
	int channelCount = channelPositions.size();

	for (int i = 0; i < channelCount; i++) {
		dynamicChannelPositions.emplace_back(); // Create one array foreach channel
	}


	for (int i = 0; i < orientations.size(); i++) {
		for (int c = 0; c < channelCount; c++) {
			static vector3 tmp;
			static const vec3_t tmpZero{ 0,0,0 };
			
			S_MixSpatialize(channelPositions[c].data,tmpZero,orientations[i].data,(vec_t*)&tmp.data);
			dynamicChannelPositions[c].push_back(tmp);
			//std::cout << tmp[0] << " " << tmp[1] << " " << tmp[2] << " \n";
		}
	}


	





	// Metadata generation
	auto admProgramme = adm::AudioProgramme::create(adm::AudioProgrammeName("RotatingADMWavGenerator Export"));
	auto admDocument = adm::Document::create();
	admDocument->add(admProgramme);

	std::vector<bw64::AudioId> audioIds;

	
	long frameTime = 1;
	long long frameDurationInNanoSeconds = S_FrameTimeToNanoSeconds(frameTime,fps);

	for (int i = 0; i < channelCount; i++) {
		std::string contentName("Dynamically Spatialized Channel");
		contentName.append(std::to_string(i));
		auto contentItem = adm::AudioContent::create(adm::AudioContentName(contentName));
		admProgramme->addReference(contentItem);

		auto channelObject = adm::createSimpleObject(contentName);

		int64_t blocksAdded = 0;

		long long lastBlockEndTimeChannelScope = 0;

		//int b = 0;
		//int bLast = object->blocks.size() - 1;
		long long thisBlockEndTime = 0, lastBlockEndTime = 0;

		for (long o=0; o < dynamicChannelPositions[i].size();o++) {
			
			long long timeInNanoSeconds = S_FrameTimeToNanoSeconds(o,fps);
			long long durationInNanoSeconds = frameDurationInNanoSeconds;

			adm::CartesianPosition cartesianCoordinates((adm::X)(dynamicChannelPositions[i][o].data[0]), (adm::Y)(dynamicChannelPositions[i][o].data[1]), (adm::Z)(dynamicChannelPositions[i][o].data[2]));
			auto blockFormat = adm::AudioBlockFormatObjects(cartesianCoordinates);
			thisBlockEndTime = timeInNanoSeconds + frameDurationInNanoSeconds; // The time that this block ends in correct numbers
			
			if (!!lastBlockEndTime && lastBlockEndTime != timeInNanoSeconds) { // It's at least the second block in this object. They are contiguous. Simply adjust times.
					// This means that we have a rounding error resulting from conversion of samples to nanoseconds
				long long diff = timeInNanoSeconds - lastBlockEndTime;
				timeInNanoSeconds -= diff; // We remove the diff from the start value
				durationInNanoSeconds += diff; // Since duration is a relative term, we need to add the diff back on
			}

			if (o == 0) {
				// If this is the first block in this sound, make the position a jumper.
				// TODO technically we are introducing a delay in the position data here because
				// the position data is for the start of the block, not for its end
				// but we'll let it slide for now... the correct way would be to 
				// add the zero-duration blockformats...

				blockFormat.set(adm::JumpPosition(adm::JumpPositionFlag(true)));
			}

			blockFormat.set(adm::Rtime((std::chrono::nanoseconds)timeInNanoSeconds));
			blockFormat.set(adm::Duration((std::chrono::nanoseconds)durationInNanoSeconds));

			lastBlockEndTimeChannelScope = lastBlockEndTime = thisBlockEndTime; // Remember for next iteration

			channelObject.audioChannelFormat->add(blockFormat);


			blocksAdded++;
		}

		if (blocksAdded > 0) {

			contentItem->addReference(channelObject.audioObject);
			audioIds.push_back(bw64::AudioId(i + 1,
				adm::formatId(channelObject.audioTrackUid->get<adm::AudioTrackUidId>()),
				adm::formatId(channelObject.audioTrackFormat->get<adm::AudioTrackFormatId>()),
				adm::formatId(channelObject.audioPackFormat->get<adm::AudioPackFormatId>())
			));
		}
	}

	std::stringstream ss;
	ss << audioFile;
	ss << ".ADM.xml";
	std::string admXMLFileName = ss.str();
	ss.str("");
	ss << audioFile;
	ss << ".ADM.rotated.wav";
	std::string bw64filename = ss.str();



	// write XML data to stdout
	//adm::writeXml(admXMLFileName, admDocument);

	auto chnaChunk = std::make_shared<bw64::ChnaChunk>(audioIds);

	// Copy over audio data.
	std::unique_ptr<bw64::Bw64Writer>	adm_bw64Handle;

	auto inFile = bw64::readFile(audioFile);

	std::cout << "Input Format:" << std::endl;
	std::cout << " - formatTag: " << inFile->formatTag() << std::endl;
	std::cout << " - channels: " << inFile->channels() << std::endl;
	std::cout << " - sampleRate: " << inFile->sampleRate() << std::endl;
	std::cout << " - bitDepth: " << inFile->bitDepth() << std::endl;
	std::cout << " - numerOfFrames: " << inFile->numberOfFrames() << std::endl;

	auto outFile = bw64::writeFile(bw64filename, inFile->channels(), inFile->sampleRate(),
		inFile->bitDepth());

	ss.str("");
	adm::writeXml(ss, admDocument);

	auto axmlCunk = std::make_shared<bw64::AxmlChunk>(ss.str());
	outFile->setAxmlChunk(axmlCunk);
	outFile->setChnaChunk(chnaChunk);


	std::vector<float> buffer(BLOCK_SIZE* inFile->channels());
	while (!inFile->eof()) {
		auto readFrames = inFile->read(&buffer[0], BLOCK_SIZE);
		outFile->write(&buffer[0], readFrames);
	}
	

	std::cin.get();
}