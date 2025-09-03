#include "gtest/gtest.h"
#include "logging/Logger.h"
#include "logging/ILoggerBackend.h"
#include "logging/Assert.h"
#include "TestSingleThreadedUtils.h"
#include <vector>
#include <string>

// Traceability: 
// PRS-96 Logger tested herein
// PRS-95 Assert handling tested herein

static uint32_t get_tick_count()
{
    return 0x12345678;
}


class LoggerTest : public testing::Test 
{
    public:
        LoggerTest()
        {
            // Self-register with the logger.
            EXPECT_TRUE(eg::Logger::register_backend(m_log_backend));
            // It's OK to register the same backend more than once. 
            EXPECT_TRUE(eg::Logger::register_backend(m_log_backend)); 
        }
        ~LoggerTest()
        {
            // We may need to add this feature just for testing.
            //eg::Logger::unregister_backend(m_log_backend);
        }

    protected:
        using LogData = std::vector<std::string>;

        // Simple backend to append log entries to a vector of strings.
        class TestBackend : public eg::ILoggerBackend
        {
            public:
                TestBackend(LogData& log_data)
                : m_log_data{log_data}
                {                
                }
                void write(const char* message, bool new_line = false)
                {
                    std::ostringstream os;
                    os << message;
                    if (new_line) 
                        os << '\n';
                    m_log_data.push_back(os.str());
                }

            private:
                LogData& m_log_data;
        };

        virtual void SetUp() 
        {
            m_log_data.clear();
        }

        virtual void TearDown() 
        {
        }

    protected:
        LogData     m_log_data;
        TestBackend m_log_backend{m_log_data};
};


TEST_F(LoggerTest, LoggingMacros)
{
    eg::Logger::register_ticker(get_tick_count);
    constexpr auto line1 = __LINE__ + 1;
    EG_LOG_DEBUG("Whatever1");
    constexpr auto line2 = __LINE__ + 1;
    EG_LOG_DEBUG("Whatever2 %u", 1234);
    constexpr auto line3 = __LINE__ + 1;
    EG_LOG_DEBUG("Whatever3 %08X", 0x1234);
    constexpr auto line4 = __LINE__ + 1;
    EG_LOG_DEBUG("Whatever4");

    //for (const auto& s: m_log_data)
    //    std::cout << s;
    //std::cout<< m_log_data.size() << std::endl;

    EXPECT_TRUE(m_log_data.size() == 4); 

    // Recreate the prefix using the line numbers cached above. Note final space. 
    // See Logger::write for formatting.
    std::string prefix("DEBUG: [84:50:19.896] TestLogger.cpp:");
    std::stringstream str1, str2, str3, str4;
    str1 << prefix << std::to_string(line1) << " (TestBody) Whatever1" << "\n" << "\n";
    str2 << prefix << std::to_string(line2) << " (TestBody) Whatever2 1234" << "\n" << "\n";
    str3 << prefix << std::to_string(line3) << " (TestBody) Whatever3 00001234" << "\n" << "\n";
    str4 << prefix << std::to_string(line4) << " (TestBody) Whatever4" << "\n" << "\n";

    EXPECT_TRUE(m_log_data[0] == str1.str()); 
    EXPECT_TRUE(m_log_data[1] == str2.str()); 
    EXPECT_TRUE(m_log_data[2] == str3.str()); 
    EXPECT_TRUE(m_log_data[3] == str4.str()); 

    // TODO_AC We want to test for error conditions. 
    // - Broken formatting. Error string output
    // - Overlong message. Truncated output
    // Maybe refactor the logger so we can create temporary instances and change 
    // the size of the formatting buffer with a template argument. How to manage the 
    // macros so they find the Logger object? Settable pointer to an ILogger? Static member 
    // set by constructor? 
}

TEST_F(LoggerTest, LoggingMacrosLevels)
{
    eg::Logger::register_ticker(get_tick_count);
    constexpr auto line1 = __LINE__ + 1;
    EG_LOG_ERROR("Error");
    constexpr auto line2 = __LINE__ + 1;
    EG_LOG_WARN("Warn");
    constexpr auto line3 = __LINE__ + 1;
    EG_LOG_INFO("Info");
    constexpr auto line4 = __LINE__ + 1;
    EG_LOG_DEBUG("Debug");
    constexpr auto line5 = __LINE__ + 1;
    EG_LOG_TRACE("Trace");
    
    EXPECT_TRUE(m_log_data.size() == 5); 

    std::string timestamp("[84:50:19.896] TestLogger.cpp:");
    std::stringstream str1, str2, str3, str4, str5;
    str1 << "ERROR: " << timestamp << std::to_string(line1) << " (TestBody) Error" << "\n" << "\n";
    str2 << "WARN:  " << timestamp << std::to_string(line2) << " (TestBody) Warn" << "\n"<< "\n";
    str3 << "INFO:  " << timestamp << std::to_string(line3) << " (TestBody) Info" << "\n"<< "\n";
    str4 << "DEBUG: " << timestamp << std::to_string(line4) << " (TestBody) Debug" << "\n"<< "\n";
    str5 << "TRACE: " << timestamp << std::to_string(line5) << " (TestBody) Trace" << "\n"<< "\n";
    EXPECT_TRUE(m_log_data[0] == str1.str()); 
    EXPECT_TRUE(m_log_data[1] == str2.str()); 
    EXPECT_TRUE(m_log_data[2] == str3.str()); 
    EXPECT_TRUE(m_log_data[3] == str4.str()); 
    EXPECT_TRUE(m_log_data[4] == str5.str()); 
}


TEST_F(LoggerTest, TooLong)
{
    eg::Logger::register_ticker(get_tick_count);
    // Long string
    std::stringstream str1;
    for (int i = 0; i < 512; i++)
        str1 << "A";
        
    EG_LOG_DEBUG("%s", str1.str().c_str());
    EXPECT_TRUE(m_log_data[0].substr(250, 6) == "A...\n\n");

    // Really long string
    std::stringstream str2;
    for (int i = 0; i < 65536; i++)
        str2 << "A";
    EG_LOG_DEBUG("%s", str2.str().c_str());
    EXPECT_TRUE(m_log_data[1].substr(250, 6) == "A...\n\n");

    // Exactly the right length
    // NB the correct length of the prefix may change if this line number or file name changes
    int prefix_length(52);
    std::stringstream str3;
    for (int i = 0; i < 254 - prefix_length; i++)
        str3 << "A";
    EG_LOG_DEBUG("%s", str3.str().c_str());
    EXPECT_TRUE(m_log_data[2].substr(250, 6) == "AAAA\n\n");
}


TEST_F(LoggerTest, Register) 
{
    eg::Logger::register_ticker(get_tick_count);
    //Test assumes max backends is four - amend this test if not
    EXPECT_TRUE(OTWAY_LOGGER_MAX_BACKENDS == 4U);

    //Fixture starts fresh and registers one backend. 
    //Register 4 more, final one will fail. 
    std::vector<std::string> log_data1, log_data2, log_data3, log_data4;
    TestBackend log_backend1(log_data1), log_backend2(log_data2), log_backend3(log_data3), log_backend4(log_data4);
    EXPECT_TRUE(eg::Logger::register_backend(log_backend1));
    EXPECT_TRUE(eg::Logger::register_backend(log_backend2));
    EXPECT_TRUE(eg::Logger::register_backend(log_backend3));
    EXPECT_FALSE(eg::Logger::register_backend(log_backend4));
}

TEST_F(LoggerTest, Raw)
{
    EG_LOG_RAW("Error");
    EXPECT_EQ(m_log_data[0], "Error\n"); 
}


TEST_F(LoggerTest, NoFileLocation)
{
    // We don't expect calls to be made without the macros, but this 
    // test covers a direct call made where no calling function is provided.
    constexpr auto line1 = __LINE__ + 1;
    eg::Logger::log<::eg::Logger::Level::Debug>(__FILE__, __LINE__, nullptr, "Error");
    EXPECT_EQ(m_log_data.size(), 1); 

    std::stringstream str1;
    str1 << "DEBUG: TestLogger.cpp:" << std::to_string(line1) << " Error\n\n";
    std::cout << m_log_data[0] << std::endl;
    std::cout << str1.str() << std::endl;
    EXPECT_TRUE(m_log_data[0] == str1.str()); 
}



static inline void example_datetime_func(eg::Logger::DateTime& output) {
    // Standing in for hardware access to date time
    output.year = 1;
    output.month = 2;
    output.day = 3;
    output.hour = 4;
    output.minute = 5;
    output.second = 6;
    output.millis = 7;
    std::cout << "example_datetime_func" << std::endl;
}


TEST_F(LoggerTest, CustomDateTime) 
{
    eg::Logger::register_datetime_source(example_datetime_func);
    constexpr auto line1 = __LINE__ + 1;
    EG_LOG_ERROR("%s", "Date time test");
    std::stringstream str1;
    str1 << "ERROR: [0001/02/03 04:05:06.007] TestLogger.cpp:" << std::to_string(line1) << " (TestBody) Date time test\n\n";
    EXPECT_EQ(m_log_data[0], str1.str()); 
}


TEST_F(LoggerTest, Assert)
{
    eg::Logger::register_datetime_source(example_datetime_func);
    // Check we log on call of assert_triggered
    constexpr auto line1 = __LINE__ + 1;
    eg::assert_triggered(__FILE__, line1, "TestBody", "assert logged sucessfully");
    std::stringstream str1;
    str1 << "FATAL: [0001/02/03 04:05:06.007] TestLogger.cpp:" << std::to_string(line1) << " (TestBody) assert logged sucessfully\n\n";
    EXPECT_EQ(m_log_data[0], str1.str()); 
    // on_assert_triggered is intended to be implemented by other targets and has no side effects in this test environment.
}

// Confirm that invalid formatting results in expected output and doesn't crash us out. 
TEST_F(LoggerTest, InvalidFormatting)
{
    eg::Logger::register_datetime_source(example_datetime_func);
    // gcc rightly gets mad at using an invalid format, so it's 
    // appropriate to locally suppress these warnings here.
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat="
    #pragma GCC diagnostic ignored "-Wformat-extra-args"
    // codechecker_intentional [core.uninitialized.Assign, clang-diagnostic-format] deliberately invalid formatting
    EG_LOG_DEBUG("%q", nullptr); 
    #pragma GCC diagnostic pop
    EXPECT_EQ(m_log_data[0], std::string("Format encoding error\n")); 
}