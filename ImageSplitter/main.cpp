#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include <Windows.h>

namespace filesystem = std::experimental::filesystem;


void displayIntro();

std::string getString(const std::string& prompt);

sf::Uint16 getInt(const std::string& prompt);

void crop(const sf::Image& sourceImage, sf::Uint16 left, sf::Uint16 top, sf::Uint16 width, sf::Uint16 height, const std::string& filename);

void split(const std::string& sourceFilename, const std::string& destFolder, sf::Uint16 padX, sf::Uint16 padY, sf::Uint16 width, sf::Uint16 height);

void splitFiles(const std::string& sourceFolder, const std::string& destFolder, sf::Uint16 padX, sf::Uint16 padY, sf::Uint16 width, sf::Uint16 height);

std::vector<std::string> getFilenames(const std::string& sourceFolder, const std::string& extension);

std::string createFilename(const std::string& sourceFilename, const std::string& destFolder, std::size_t numberLength, sf::Uint16 fileNumber);


int main() {

	displayIntro();

	std::string sourceFolder = getString("Enter folder with source images: ");
	std::string destFolder = getString("Enter folder for destination images (creates folder if it doesn't exist): ");
	sf::Uint16 padX = getInt("Enter padding in X direction (0 if none): ");
	sf::Uint16 padY = getInt("Enter padding in Y direction (0 if none): ");
	sf::Uint16 width = getInt("Enter width of split images: ");
	sf::Uint16 height = getInt("Enter height of split images: ");

	std::cout << "Splitting..." << std::endl;
	sf::sleep(sf::milliseconds(150));

	splitFiles(sourceFolder, destFolder, padX, padY, width, height);

	return EXIT_SUCCESS;
}



void displayIntro() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	const sf::Uint16 columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	const std::string title("Welcome to Image Splitter!");
	const sf::Uint16 diff = columns - title.length();
	const sf::Uint16 padLeft = diff / 2;
	const sf::Uint16 padRight = diff - padLeft;
	const std::string titleCentered = std::string(padLeft, ' ') + title + std::string(padRight, ' ');
	
	std::cout << titleCentered << std::endl << std::endl;
}


std::string getString(const std::string& prompt) {
	std::cout << prompt;
	std::string str;
	std::cin >> str;
	return str;
}


sf::Uint16 getInt(const std::string& prompt) {
	std::cout << prompt;
	sf::Uint16 newInt;
	std::cin >> newInt;
	return newInt;
}


void crop(const sf::Image& sourceImage, sf::Uint16 left, sf::Uint16 top, sf::Uint16 width, sf::Uint16 height, const std::string& filename) {
	const sf::IntRect sourceRect(left, top, width, height);

	sf::Image imagePart;
	imagePart.create(width, height, sf::Color::Transparent);
	imagePart.copy(sourceImage, 0, 0, sourceRect, true);

	if (imagePart.saveToFile(filename))
		std::cout << "Saved image to " << filename << '.' << std::endl;
}


void split(const std::string& sourceFilename, const std::string& destFolder, sf::Uint16 padX, sf::Uint16 padY, sf::Uint16 width, sf::Uint16 height) {
	sf::Image sourceImage;
	sourceImage.loadFromFile(sourceFilename);

	const sf::Uint32 columns = sourceImage.getSize().x / (width + padX * 2);
	const sf::Uint32 rows = sourceImage.getSize().y / (height + padY * 2);
	const std::size_t numberLength = std::to_string(columns * rows).length();

	sf::Uint16 fileNumber = 0;
	sf::Uint16 offsetY = padY;

	for (sf::Uint16 row = 0; row < rows; ++row) {
		sf::Uint16 offsetX = padX;

		for (sf::Uint16 col = 0; col < columns; ++col) {
			std::string destFilename = createFilename(sourceFilename, destFolder, numberLength, fileNumber);
			crop(sourceImage, offsetX + (col * width), offsetY + (row * height), width, height, destFilename);
			offsetX += 2 * padX;
			fileNumber++;
		}
		offsetY += 2 * padY;
	}
}


void splitFiles(const std::string& sourceFolder, const std::string& destFolder, sf::Uint16 padX, sf::Uint16 padY, sf::Uint16 width, sf::Uint16 height) {
	// If folder is not found, create folder and tell user
	if (!filesystem::is_directory(destFolder)) {
		if (filesystem::create_directory(destFolder))
			std::cout << destFolder << " folder created." << std::endl;
	}

	const std::string extension = ".png";
	std::vector<std::string> filenames = getFilenames(sourceFolder, extension);

	for (const std::string& filename : filenames) {
		split(filename, destFolder, padX, padY, width, height);
	}

}


std::vector<std::string> getFilenames(const std::string& sourceFolder, const std::string& extension) {
	std::vector<std::string> filenames;
	for (const auto& entry : filesystem::directory_iterator(sourceFolder)) {
		if (extension.empty() || entry.path().extension() == extension) {
			filenames.push_back(entry.path().string());
		}
	}

	return filenames;
}


std::string createFilename(const std::string& sourceFilename, const std::string& destFolder, std::size_t numberLength, sf::Uint16 fileNumber) {
	filesystem::path sourcePath(sourceFilename);
	filesystem::path rawFilenamePath(destFolder);
	rawFilenamePath /= sourcePath.stem();

	std::stringstream destFilename;
	destFilename << rawFilenamePath.string();
	destFilename << std::setw(numberLength) << std::setfill('0') << fileNumber;
	//TODO: Change this so it can accept other filetypes as well
	destFilename << ".png";

	return destFilename.str();
}
