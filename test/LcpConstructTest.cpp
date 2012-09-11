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
sdsl::tMSS global_file_map;

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
//             test_cases.push_back(test_cases_dir + "/crafted/100a.txt");
//             test_cases.push_back(test_cases_dir + "/crafted/abc_abc_abc.txt");
//             test_cases.push_back(test_cases_dir + "/crafted/abc_abc_abc2.txt");
//             test_cases.push_back(test_cases_dir + "/crafted/empty.txt");
            test_cases.push_back(test_cases_dir + "/crafted/example01.txt");
            test_cases.push_back(test_cases_dir + "/small/faust.txt");
            test_cases.push_back(test_cases_dir + "/small/zarathustra.txt");
        }

        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        std::vector<std::string> test_cases;
};

//! Prepare LcpConstructTest: Create text, sa and bwt of the test files
TEST_F(LcpConstructTest, prepare)
{
    string dir = "";
    sdsl::tMSS file_map;
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
            file_map["text"] = (dir+"text_"+id).c_str();
            delete [] text;
        }

        // Create Suffix-Array
        {
            sdsl::int_vector_file_buffer<8> text_buf( file_map["text"].c_str() );
            text_buf.reset();
            size_type n = text_buf.int_vector_size;

            unsigned char *text = NULL;
            ASSERT_EQ(true, sdsl::util::load_from_int_vector_buffer(text, text_buf));
            sdsl::int_vector<> sa = sdsl::int_vector<>(n, 0, sdsl::bit_magic::l1BP(n+1)+1);
            sdsl::algorithm::calculate_sa(text,n, sa);
            assert(sa.size() == n);
            ASSERT_EQ(true, sdsl::util::store_to_file(sa, (dir+"sa_"+id).c_str() ));
            file_map["sa"] = dir+"sa_"+id;
            {
                sa.resize(0);
                sdsl::int_vector<> temp;
                temp.swap(sa);
            }
        }

        // Construct BWT
        {
            ASSERT_EQ(true, sdsl::construct_bwt(file_map, "", id));
        }

        // Construct LCP-Array
        {
            ASSERT_EQ(true, sdsl::construct_lcp_kasai(file_map, dir, "org_" + id));
        }

        // Save needed data structures and delete not needed data structes
        global_file_map["text_"+id] = file_map["text"];
        file_map.erase("text");
        global_file_map["sa_"+id]   = file_map["sa"];
        file_map.erase("sa");
        global_file_map["bwt_"+id]  = file_map["bwt"];
        file_map.erase("bwt");
        global_file_map["lcp_"+id]  = file_map["lcp"];
        file_map.erase("lcp");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_semi_extern_PHI
TEST_F(LcpConstructTest, construct_lcp_semi_extern_PHI)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_semi_extern_PHI(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_PHI
TEST_F(LcpConstructTest, construct_lcp_PHI)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_PHI(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_simple_5n
TEST_F(LcpConstructTest, construct_lcp_simple_5n)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_simple_5n(file_map, dir, id);

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_simple2_9n
TEST_F(LcpConstructTest, construct_lcp_simple2_9n)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_simple2_9n(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_go
TEST_F(LcpConstructTest, construct_lcp_go)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_go(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_goPHI
TEST_F(LcpConstructTest, construct_lcp_goPHI)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_goPHI(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}


//! Test LCP-Construction construct_lcp_go2
TEST_F(LcpConstructTest, construct_lcp_go2)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_go2(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}


//! Test LCP-Construction construct_lcp_bwt_based
TEST_F(LcpConstructTest, construct_lcp_bwt_based)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_bwt_based(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Test LCP-Construction construct_lcp_bwt_based2
TEST_F(LcpConstructTest, construct_lcp_bwt_based2)
{
    string dir = "";
    sdsl::tMSS file_map;
    for (size_t i=0; i< this->test_cases.size(); ++i) {

        // Prepare LCP-Array construction
        string id = "tc_"+sdsl::util::to_string(i);
        file_map["text"] = global_file_map["text_"+id];
        file_map["sa"]   = global_file_map["sa_"+id];
        file_map["bwt"]  = global_file_map["bwt_"+id];

        // Construct LCP-Array
        ASSERT_EQ(true, sdsl::construct_lcp_bwt_based2(file_map, dir, id));

        // Check LCP-Array
        sdsl::int_vector<> lcp1, lcp2;
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp1, global_file_map["lcp_"+id].c_str()));
        ASSERT_EQ(true, sdsl::util::load_from_file(lcp2, file_map["lcp"].c_str()));
        ASSERT_EQ(lcp1.size(), lcp2.size());
        for(size_type i=0; i<lcp1.size() and i<lcp2.size(); ++i) {
            ASSERT_EQ(lcp1[i], lcp2[i]);
        }

        // Clean up everything
        file_map.erase("text");
        file_map.erase("sa");
        file_map.erase("bwt");
        sdsl::util::delete_all_files(file_map);
    }
}

//! Clean up LcpConstructTest: Delete text, sa and bwt of the test files
TEST_F(LcpConstructTest, cleanup)
{
    sdsl::util::delete_all_files(global_file_map);
}

}  // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
