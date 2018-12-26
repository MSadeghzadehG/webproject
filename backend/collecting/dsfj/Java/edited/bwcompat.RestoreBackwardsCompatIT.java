
package org.elasticsearch.bwcompat;

import org.elasticsearch.common.io.FileTestUtils;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.repositories.fs.FsRepository;
import org.elasticsearch.snapshots.AbstractSnapshotIntegTestCase;
import org.elasticsearch.snapshots.SnapshotRestoreException;
import org.elasticsearch.snapshots.mockstore.MockRepository;
import org.elasticsearch.test.ESIntegTestCase.ClusterScope;
import org.elasticsearch.test.ESIntegTestCase.Scope;
import org.junit.BeforeClass;

import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertAcked;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.greaterThan;


@ClusterScope(scope = Scope.TEST)
public class RestoreBackwardsCompatIT extends AbstractSnapshotIntegTestCase {

    private static Path repoPath;

    @Override
    protected Settings nodeSettings(int nodeOrdinal) {
        return Settings.builder()
            .put(super.nodeSettings(nodeOrdinal))
            .put(Environment.PATH_REPO_SETTING.getKey(), repoPath)
            .build();
    }

    @BeforeClass
    public static void repoSetup() throws IOException {
        repoPath = createTempDir("repositories");
    }

    public void testRestoreUnsupportedSnapshots() throws Exception {
        String repo = "test_repo";
        String snapshot = "test_1";
        List<String> repoVersions = unsupportedRepoVersions();
        assertThat(repoVersions.size(), greaterThan(0));
        for (String version : repoVersions) {
            createRepo("unsupportedrepo", version, repo);
            assertUnsupportedIndexFailsToRestore(repo, snapshot);
        }
    }

    private List<String> unsupportedRepoVersions() throws Exception {
        return listRepoVersions("unsupportedrepo");
    }

    private List<String> listRepoVersions(String prefix) throws Exception {
        List<String> repoVersions = new ArrayList<>();
        Path repoFiles = getBwcIndicesPath();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(repoFiles, prefix + "-*.zip")) {
            for (Path entry : stream) {
                String fileName = entry.getFileName().toString();
                String version = fileName.substring(prefix.length() + 1);
                version = version.substring(0, version.length() - ".zip".length());
                repoVersions.add(version);
            }
        }
        return repoVersions;
    }

    private void createRepo(String prefix, String version, String repo) throws Exception {
        Path repoFileFromBuild = getBwcIndicesPath().resolve(prefix + "-" + version + ".zip");
        String repoFileName = repoFileFromBuild.getFileName().toString().split(".zip")[0];
        Path fsRepoPath = repoPath.resolve(repoFileName);
        FileTestUtils.unzip(repoFileFromBuild, fsRepoPath, null);
        logger.info("-->  creating repository [{}] for version [{}]", repo, version);
        assertAcked(client().admin().cluster().preparePutRepository(repo)
            .setType(MockRepository.TYPE).setSettings(Settings.builder()
                .put(FsRepository.REPOSITORIES_LOCATION_SETTING.getKey(), fsRepoPath.getParent().relativize(fsRepoPath).resolve("repo").toString())));
    }

    private void assertUnsupportedIndexFailsToRestore(String repo, String snapshot) throws IOException {
        logger.info("--> restoring unsupported snapshot");
        try {
            client().admin().cluster().prepareRestoreSnapshot(repo, snapshot).setRestoreGlobalState(true).setWaitForCompletion(true).get();
            fail("should have failed to restore - " + repo);
        } catch (SnapshotRestoreException ex) {
            assertThat(ex.getMessage(), containsString("snapshot does not exist"));
        }
    }
}

