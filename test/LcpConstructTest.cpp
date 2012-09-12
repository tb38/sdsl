#include <sdsl/suffixarrays.hpp>
#include <sdsl/lcp_construct.hpp>
#include <sdsl/bwt_construct.hpp>
#include "sdsl/config.hpp" // for CMAKE_SOURCE_DIR
#include "gtest/gtest.h"
#include <vector>
#include <string>

namespace
{

typedef sdsl::int_vector<>::size_type size_type;
typedef std::map<std::string, bool (*)(sdsl::tMSS&, const std::string&, const std::string&)> tMSFP;


// The fixture for testing class int_vector.
class LcpConstructTest : public ::testing::Test
{
    protected:

        LcpConstructTest() {
            // You can do set-up work for each test here.
        }

        virtual ~LcpConstructTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
            string test_cases_dir = string(SDSL_XSTR(CMAKE_SOURCE_DIR)) + "/test/test_cases";
            test_cases.push_back(test_cases_dir + "/crafted/100a.txt");
//             test_cases.push_back(test_cases_dir + "/crafted/abc_abc_abc.txt");
//             test_cases.push_back(test_cases_dir + "/crafted/abc_abc_abc2.txt"); //TODO check the problem with this one
            test_cases.push_back(test_cases_dir + "/crafted/empty.txt");
            test_cases.push_back(test_cases_dir + "/crafted/example01.txt");
            test_cases.push_back(test_cases_dir + "/small/faust.txt");
            test_cases.push_back(test_cases_dir + "/small/zarathustra.txt");

// 			lcp_function["construct_lcp_semi_extern_PHI"] = &sdsl::construct_lcp_semi_extern_PHI; // TODO: Handle empty test case
// 			lcp_function["construct_lcp_PHI"] = &sdsl::construct_lcp_PHI; TODO: Handle default argument
			lcp_function["construct_lcp_simple2_9n"] = &sdsl::construct_lcp_simple2_9n;
			lcp_function["construct_lcp_go"] = &sdsl::construct_lcp_go;
			lcp_function["construct_lcp_goPHI"] = &sdsl::construct_lcp_goPHI;
			lcp_function["construct_lcp_go2"] = &sdsl::construct_lcp_go2;
			lcp_function["construct_lcp_bwt_based"] = &sdsl::construct_lcp_bwt_based;
			lcp_function["construct_lcp_bwt_based2"] = &sdsl::construct_lcp_bwt_based2;


			string dir = "";
			sdsl::tMSS tmp_file_map;
			for (size_t i=0; i< this->test_cases.size(); ++i) {
				string id = "tc_"+sdsl::util::to_string(i);
				// Prepare Input
				{
					sdsl::int_vector_file_buffer<8> text_buf;
					text_buf.load_from_plain(this->test_cases[i].c_str());
					text_buf.reset();
					unsigned char *text = NULL;
					ASSERT_EQ(true, sdsl::util::load_from_int_vector_buffer(text, text_buf));
					size_type n = text_buf.int_vector_size;
					ASSERT_EQ(true, sdsl::util::store_to_file(sdsl::char_array_serialize_wrapper<>((unsigned char*)text,n+1), (dir+"text_"+id).c_str() ));
					tmp_file_map["text"] = (dir+"text_"+id).c_str();
					delete [] text;
				}

				// Create Suffix-Array
				{
					sdsl::int_vector_file_buffer<8> text_buf( tmp_file_map["text"].c_str() );
					text_buf.reset();
					size_type n = text_buf.int_vector_size;

					unsigned char *text = NULL;
					ASSERT_EQ(true, sdsl::util::load_from_int_vector_buffer(text, text_buf));
					sdsl::int_vector<> sa = sdsl::int_vector<>(n, 0, sdsl::bit_magic::l1BP(n+1)+1);
					sdsl::algorithm::calculate_sa(text,n, sa);
					assert(sa.size() == n);
					ASSERT_EQ(true, sdsl::util::store_to_file(sa, (dir+"sa_"+id).c_str() ));
					tmp_file_map["sa"] = dir+"sa_"+id;
					{
						sa.resize(0);
						sdsl::int_vector<> temp;
						temp.swap(sa);
					}
				}

				// Construct BWT
				{
					ASSERT_EQ(true, sdsl::construct_bwt(tmp_file_map, "", id));
				}

				// Construct LCP-Array
				{
					ASSERT_EQ(true, sdsl::construct_lcp_kasai(tmp_file_map, dir, "org_" + id));
				}

				// Save needed data structures and delete not needed data structes
				file_map["text_"+id] = tmp_file_map["text"];
				tmp_file_map.erase("text");
				file_map["sa_"+id]   = tmp_file_map["sa"];
				tmp_file_map.erase("sa");
				file_map["bwt_"+id]  = tmp_file_map["bwt"];
				tmp_file_map.erase("bwt");
				file_map["lcp_"+id]  = tmp_file_map["lcp"];
				tmp_file_map.erase("lcp");
				sdsl::util::delete_all_files(tmp_file_map);
			}
		}

        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
			sdsl::util::delete_all_files(file_map);
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        std::vector<std::string> test_cases;
		sdsl::tMSS file_map;
		tMSFP lcp_function;
};

TEST_F(LcpConstructTest, construct_lcp)
{
    string dir = "";
	for (tMSFP::const_iterator it = this->lcp_function.begin(), end = this->lcp_function.end(); it != end; ++it) {
		sdsl::tMSS file_map;
		for (size_t i=0; i< this->test_cases.size(); ++i) {
// 			std::cout << (it->first) << " on test file " << this->test_cases[i] << std::endl;

			// Prepare LCP-Array construction
			string id = "tc_"+sdsl::util::to_string(i);
			file_map["text"] = this->file_map["text_"+id];
			file_map["sa"]   = this->file_map["sa_"+id];
			file_map["bwt"]  = this->file_map["bwt_"+id];

			// Construct LCP-Array
			ASSERT_EQ(true, (it->second)(file_map, dir, id)) 
					<< (it->first) << " on test file " << this->test_cases[i] << "was not successfull.";

			// Check LCP-Array
			sdsl::int_vector<> lcp1, lcp2;
			ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, this->file_map["lcp_"+id].c_str()))
					<< (it->first) << " on test file " << this->test_cases[i] << " could not load reference lcp array";
			ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()))
					<< (it->first) << " on test file " << this->test_cases[i] << " could not load created lcp array";
			ASSERT_EQ(lcp1.size(), lcp2.size())
					<< (it->first) << " on test file " << this->test_cases[i] << " size differ";
			for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
				ASSERT_EQ(lcp1[i], lcp2[i])
					<< (it->first) << " on test file " << this->test_cases[i] << " value differ:"
					<< " lcp1[" << i << "]=" << lcp1[i] << "!=" << lcp2[i] << "=lcp2["<< i << "]";
			}

			// Clean up everything
			file_map.erase("text");
			file_map.erase("sa");
			file_map.erase("bwt");
			sdsl::util::delete_all_files(file_map);
		}
	}
}

}  // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
