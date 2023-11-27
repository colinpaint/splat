//{{{  includes
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstdint>
#include <algorithm>

using namespace std;
//}}}
//{{{
inline void write_hex(ofstream& out, uint8_t c)
{
		const char base16_digits[] = "0123456789abcdef";

		out << "0x";
		if (c < 16)
		{
				out << "0";
				out << base16_digits[c];
		}
		else
		{
				out << base16_digits[c >> 4];
				out << base16_digits[c & 0xf];
		}
}
//}}}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		//{{{  error, return
		cerr << "USAGE: " << argv[0] << " {sym} {rsrc}\n\n"
				 << "  Creates {sym}.cpp from the contents of {rsrc}\n" << endl;
		return EXIT_FAILURE;
		}
		//}}}

	string sym(argv[1]), rsrc(argv[2]);

	ifstream in (rsrc, ifstream::binary);
	if (in.fail()) {
		//{{{  error,return
		cerr << "Could not open file " << rsrc << " for reading." << endl;
		exit(EXIT_FAILURE);
		}
		//}}}

	ofstream out(sym + ".cpp");
	if (out.fail()) {
		//{{{  error, return
		cerr << "Could not open file " << sym << " for writing." << endl;
		exit(EXIT_FAILURE);
		}
		//}}}

	out << "#include <cstdlib>\n\n";
	out << "extern const unsigned char " << sym << "[] = {";

	in.seekg (0, in.end);
	size_t size = in.tellg();
	in.seekg (0, in.beg);

	char buf[1024];
	size_t i(0);

	while (size > 0) {
	size_t n = min (size, static_cast<size_t>(1024));
		in.read(buf, n);
		size = size - n;

		if (in.fail()) {
			//{{{  error, return
			cerr << "Could not read from file." << endl;
			exit(EXIT_FAILURE);
			}
			//}}}

		for (unsigned int j(0); j < static_cast<unsigned int>(n); ++j) {
			if (i % 12 == 0) 
				out << "\n    ";

			write_hex (out, *reinterpret_cast<uint8_t*>(&buf[j]));
			out << ", ";

			++i;
			}
		}

	out << "0x0 };\n\n";
	out << "const size_t " << sym << "_len = sizeof(" << sym << ");\n\n";

	in.close();
	out.close();

	return EXIT_SUCCESS;
	}
