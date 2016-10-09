#include "../src/engine.h"
#include <gtest/gtest.h>
class EngineTests: public ::testing::Test {
    protected: 
        virtual void SetUp(){
            static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
            plog::init(plog::debug, &consoleAppender);
        }
    Engine e;
};

TEST_F(EngineTests, BasicSecrets) {
    ASSERT_TRUE(e.search_for_content_match("password", 1, "/cleartextpassword", ""));
    ASSERT_TRUE(e.search_for_content_match("credential", 1, "/cleartextcredential", ""));
    ASSERT_TRUE(e.search_for_content_match("abcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRS==", 1, "/azurekey", ""));
    ASSERT_TRUE(e.search_for_content_match("abcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRSabcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRS==", 1, "/azurekey", ""));
    ASSERT_TRUE(e.search_for_content_match("abcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRS", 1, "/azurekey", ""));
    ASSERT_TRUE(e.search_for_content_match("abcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRSabcdefghijklmonpqrstyvxzABCDEFGHIJKLMONPQRS", 1, "/azurekey", ""));
    ASSERT_TRUE(e.search_for_content_match("CryptDeriveKeyafhafaf", 1, "/decryptkey", ""));
    ASSERT_TRUE(e.search_for_content_match("CryptGenKeyajfaf", 1, "/encryptkey", ""));
    ASSERT_TRUE(e.search_for_content_match("HMACSHA1", 1, "/machinekey", ""));
    ASSERT_TRUE(e.search_for_content_match("machinekey", 1, "/machinekey", ""));
}

TEST_F(EngineTests,RealisticFileNames) { 
    ASSERT_TRUE(e.search_for_filename_match("prod.pfx",""));
    ASSERT_TRUE(e.search_for_filename_match("test.cscfg",""));
    ASSERT_TRUE(e.search_for_filename_match("doesntmatter.pubxml",""));
    ASSERT_TRUE(e.search_for_filename_match("user.password",""));
    ASSERT_TRUE(e.search_for_filename_match("id_rsa",""));
    ASSERT_TRUE(e.search_for_filename_match("id_dsa",""));
    ASSERT_TRUE(e.search_for_filename_match("id_ed25519",""));
    ASSERT_TRUE(e.search_for_filename_match("id_ecdsa",""));
    ASSERT_TRUE(e.search_for_filename_match("canbeanything.pem",""));
    ASSERT_TRUE(e.search_for_filename_match("hotsauce.pkcs12",""));
    ASSERT_TRUE(e.search_for_filename_match("pwnd.p12",""));
    ASSERT_TRUE(e.search_for_filename_match("asciiiiii.asc",""));
    ASSERT_TRUE(e.search_for_filename_match("itspublicforareason.pub",""));
    ASSERT_TRUE(e.search_for_filename_match("credentialslongstringpleaseyes.xml",""));
    ASSERT_TRUE(e.search_for_filename_match("itsafile.sdf",""));
    ASSERT_TRUE(e.search_for_filename_match("injection.sql",""));
    ASSERT_TRUE(e.search_for_filename_match("embeddeddevice.sqlite",""));
    ASSERT_TRUE(e.search_for_filename_match("mysql_historyrawrarwar",""));
    ASSERT_TRUE(e.search_for_filename_match("psql_historywtf",""));
    ASSERT_TRUE(e.search_for_filename_match("dbeaver-data-sources.xml",""));
    ASSERT_TRUE(e.search_for_filename_match("databaselolol.yml",""));
    ASSERT_TRUE(e.search_for_filename_match("developererrorshappen.pcap",""));
    ASSERT_TRUE(e.search_for_filename_match("otr.private_key",""));
    ASSERT_TRUE(e.search_for_filename_match(".zsh_history",""));
    ASSERT_TRUE(e.search_for_filename_match(".bash_history",""));
    ASSERT_TRUE(e.search_for_filename_match(".irb_history",""));
    ASSERT_TRUE(e.search_for_filename_match("accounts.xml",""));
    ASSERT_TRUE(e.search_for_filename_match("xchat2.conf",""));
    ASSERT_TRUE(e.search_for_filename_match("irssi.config",""));
    ASSERT_TRUE(e.search_for_filename_match("recon-ng.db",""));
    ASSERT_TRUE(e.search_for_filename_match(".muttrc",""));
    ASSERT_TRUE(e.search_for_filename_match("whatisthis.s3cfg",""));
    ASSERT_TRUE(e.search_for_filename_match("yay.trc",""));
    ASSERT_TRUE(e.search_for_filename_match(".ovpn",""));
    ASSERT_TRUE(e.search_for_filename_match(".bashrc",""));
    ASSERT_TRUE(e.search_for_filename_match("zshrc",""));
    ASSERT_TRUE(e.search_for_filename_match("bash_profile",""));
    ASSERT_TRUE(e.search_for_filename_match("bash_aliases",""));
    ASSERT_TRUE(e.search_for_filename_match("zsh_profile",""));
    ASSERT_TRUE(e.search_for_filename_match("zsh_aliases",""));
    ASSERT_TRUE(e.search_for_filename_match("secret_token.rb",""));
    ASSERT_TRUE(e.search_for_filename_match("omniauth.rb",""));
    ASSERT_TRUE(e.search_for_filename_match("carrierwave.rb",""));
    ASSERT_TRUE(e.search_for_filename_match("schema.rb",""));
    ASSERT_TRUE(e.search_for_filename_match("settings.py",""));
    ASSERT_TRUE(e.search_for_filename_match("config.php",""));
    ASSERT_TRUE(e.search_for_filename_match("yay.kdb",""));
    ASSERT_TRUE(e.search_for_filename_match("mine.agilekeychain",""));
    ASSERT_TRUE(e.search_for_filename_match("whywouldthisbeinhere.keychain",""));
    ASSERT_TRUE(e.search_for_filename_match("checkitson.backup",""));
    ASSERT_TRUE(e.search_for_filename_match("tooshorto.dump",""));
    ASSERT_TRUE(e.search_for_filename_match("jenkins.plugins.publish_over_ssh.xml",""));
    ASSERT_TRUE(e.search_for_filename_match(".htpasswd",""));
    ASSERT_TRUE(e.search_for_filename_match("LocalSettings.php",""));
    ASSERT_TRUE(e.search_for_filename_match(".gem/credentials",""));
    ASSERT_TRUE(e.search_for_filename_match("ssh_config",""));
    ASSERT_TRUE(e.search_for_filename_match("sshd_config",""));
}
 
