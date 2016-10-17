package io.realm;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import io.realm.log.LogLevel;
import io.realm.log.RealmLog;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertNull;

@RunWith(AndroidJUnit4.class)
public class RealmLogTests {

    @Before
    public void setUp() {
        Realm.init(InstrumentationRegistry.getTargetContext());
    }

    @Test
    public void add_remove() {
        TestHelper.TestLogger testLogger = new TestHelper.TestLogger();
        RealmLog.add(testLogger);
        RealmLog.fatal("TEST");
        assertEquals("TEST", testLogger.message);
        RealmLog.remove(testLogger);
        RealmLog.fatal("TEST_AGAIN");
        assertEquals("TEST", testLogger.message);
    }

    @Test
    public void set_get_logLevel() {
        TestHelper.TestLogger testLogger = new TestHelper.TestLogger();
        RealmLog.add(testLogger);

        RealmLog.setLevel(LogLevel.FATAL);
        assertEquals(LogLevel.FATAL, RealmLog.getLevel());
        RealmLog.debug("TEST_DEBUG");
        assertNull(testLogger.message);

        RealmLog.setLevel(LogLevel.DEBUG);
        RealmLog.debug("TEST_DEBUG");
        assertEquals("TEST_DEBUG", testLogger.message);
        RealmLog.fatal("TEST_FATAL");
        assertEquals("TEST_FATAL", testLogger.message);

        RealmLog.remove(testLogger);
    }
}