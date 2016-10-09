#include "../src/gitengine.h"
#include "../src/engine.h"
#include <gtest/gtest.h>
class GitEngineTests: public ::testing::Test {
    protected: 
        virtual void SetUp(){
        }
        virtual void TearDown(){
            system("rm -r /tmp/pwndeloreantests");
        }
    GitEngine git;
};


TEST_F(GitEngineTests, RealisticFileNamesGitEngine) {
        git.remote_start("https://github.com/pegleg2060/PwnDeloreanTestRepo.git", "/tmp/pwndeloreantests");
        ASSERT_NE(git.engine.filename_matches.count("prod.pfx"),0);
        ASSERT_NE(git.engine.filename_matches.count("test.cscfg"),0);
        ASSERT_NE(git.engine.filename_matches.count("doesntmatter.pubxml"),0);
        ASSERT_NE(git.engine.filename_matches.count("user.password"),0);
        ASSERT_NE(git.engine.filename_matches.count("id_rsa"),0);
        ASSERT_NE(git.engine.filename_matches.count("id_dsa"),0);
        ASSERT_NE(git.engine.filename_matches.count("id_ed25519"),0);
        ASSERT_NE(git.engine.filename_matches.count("id_ecdsa"),0);
        ASSERT_NE(git.engine.filename_matches.count("canbeanything.pem"),0);
        ASSERT_NE(git.engine.filename_matches.count("hotsauce.pkcs12"),0);
        ASSERT_NE(git.engine.filename_matches.count("pwnd.p12"),0);
        ASSERT_NE(git.engine.filename_matches.count("asciiiiii.asc"),0);
        ASSERT_NE(git.engine.filename_matches.count("itspublicforareason.pub"),0);
        ASSERT_NE(git.engine.filename_matches.count("credentialslongstringpleaseyes.xml"),0);
        ASSERT_NE(git.engine.filename_matches.count("itsafile.sdf"),0);
        ASSERT_NE(git.engine.filename_matches.count("injection.sql"),0);
        ASSERT_NE(git.engine.filename_matches.count("embeddeddevice.sqlite"),0);
        ASSERT_NE(git.engine.filename_matches.count("mysql_historyrawrarwar"),0);
        ASSERT_NE(git.engine.filename_matches.count("psql_historywtf"),0);
        ASSERT_NE(git.engine.filename_matches.count("dbeaver-data-sources.xml"),0);
        ASSERT_NE(git.engine.filename_matches.count("databaselolol.yml"),0);
        ASSERT_NE(git.engine.filename_matches.count("developererrorshappen.pcap"),0);
        ASSERT_NE(git.engine.filename_matches.count("otr.private_key"),0);
        ASSERT_NE(git.engine.filename_matches.count(".zsh_history"),0);
        ASSERT_NE(git.engine.filename_matches.count(".bash_history"),0);
        ASSERT_NE(git.engine.filename_matches.count(".irb_history"),0);
        ASSERT_NE(git.engine.filename_matches.count("accounts.xml"),0);
        ASSERT_NE(git.engine.filename_matches.count("xchat2.conf"),0);
        ASSERT_NE(git.engine.filename_matches.count("irssi.config"),0);
        ASSERT_NE(git.engine.filename_matches.count("recon-ng.db"),0);
        ASSERT_NE(git.engine.filename_matches.count(".muttrc"),0);
        ASSERT_NE(git.engine.filename_matches.count("whatisthis.s3cfg"),0);
        ASSERT_NE(git.engine.filename_matches.count("yay.trc"),0);
        ASSERT_NE(git.engine.filename_matches.count(".ovpn"),0);
        ASSERT_NE(git.engine.filename_matches.count(".bashrc"),0);
        ASSERT_NE(git.engine.filename_matches.count("zshrc"),0);
        ASSERT_NE(git.engine.filename_matches.count("bash_profile"),0);
        ASSERT_NE(git.engine.filename_matches.count("bash_aliases"),0);
        ASSERT_NE(git.engine.filename_matches.count("zsh_profile"),0);
        ASSERT_NE(git.engine.filename_matches.count("zsh_aliases"),0);
        ASSERT_NE(git.engine.filename_matches.count("secret_token.rb"),0);
        ASSERT_NE(git.engine.filename_matches.count("omniauth.rb"),0);
        ASSERT_NE(git.engine.filename_matches.count("carrierwave.rb"),0);
        ASSERT_NE(git.engine.filename_matches.count("schema.rb"),0);
        ASSERT_NE(git.engine.filename_matches.count("settings.py"),0);
        ASSERT_NE(git.engine.filename_matches.count("config.php"),0);
        ASSERT_NE(git.engine.filename_matches.count("yay.kdb"),0);
        ASSERT_NE(git.engine.filename_matches.count("mine.agilekeychain"),0);
        ASSERT_NE(git.engine.filename_matches.count("whywouldthisbeinhere.keychain"),0);
        ASSERT_NE(git.engine.filename_matches.count("checkitson.backup"),0);
        ASSERT_NE(git.engine.filename_matches.count("tooshorto.dump"),0);
        ASSERT_NE(git.engine.filename_matches.count("jenkins.plugins.publish_over_ssh.xml"),0);
        ASSERT_NE(git.engine.filename_matches.count(".htpasswd"),0);
        ASSERT_NE(git.engine.filename_matches.count("LocalSettings.php"),0);
        //ASSERT_NE(git.engine.filename_matches.count(".gem/credentials"),0);        //TODO:FIX THIS PATTERN
        ASSERT_NE(git.engine.filename_matches.count("ssh_config"),0);
        ASSERT_NE(git.engine.filename_matches.count("sshd_config"),0);

}

