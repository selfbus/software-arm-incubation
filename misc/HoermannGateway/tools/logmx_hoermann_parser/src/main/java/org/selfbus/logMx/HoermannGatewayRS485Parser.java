package org.selfbus.logMx;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.lightysoft.logmx.business.ParsedEntry;
import com.lightysoft.logmx.mgr.LogFileParser;

/**
 * LogMX Parser for Hoermann Gateway RS485 bus logs.
 * <p>
 * Log line format:
 *   "YYYY-MM-DD HH:MM:SS.mmm" Tx/Rx;frame;count;delta ms;crc status;frame crc;calculated crc
 * <p>
 * Example:
 * "2026-04-17 16:41:42.530" Rx;68 02 01 80 96;5;35;ok;96;96
 * "2026-04-17 16:41:42.610" Rx;00 01 02 CA;4;35;ok;CA;CA
 * "2026-04-17 16:41:42.658" Rx;67 02 01 80 44;5;35;ok;44;44
 * "2026-04-17 16:41:42.753" Rx;00 01 02 CA;4;35;ok;CA;CA
 * "2026-04-17 16:41:42.784" Rx;66 02 01 80 52;5;35;ok;52;52
 * "2026-04-17 16:41:42.880" Rx;00 01 02 CA;4;35;ok;CA;CA
 * "2026-04-17 16:41:42.928" Rx;65 02 01 80 68;5;35;ok;68;68
 * "2026-04-17 16:41:43.023" Rx;00 01 02 CA;4;35;ok;CA;CA
 */
@SuppressWarnings("unused")
public class HoermannGatewayRS485Parser extends LogFileParser {

    /** Current parsed log entry */
    private ParsedEntry entry = null;

    /** Line counter – first 2 lines (file header) are skipped */
    private int lineCount = 0;

    /** Entry date formatter */
    private SimpleDateFormat dateFormat = null;

    /** Mutex for non-thread-safe SimpleDateFormat */
    private final Object DATE_FORMATTER_MUTEX = new Object();

    /** Regex: "DATE" Tx/Rx;frame;count;delta ms;crc status;frame crc;calculated crc */
    private static final Pattern ENTRY_PATTERN = Pattern.compile(
        "^\"([^\"]+)\"\\s+([^;]+);([^;]*);([^;]+);([^;]+);([^;]+);([^;]+);([^;]+)\\s*"
    );

    // Extra field keys
    private static final String FIELD_COUNT     = "Count";
    private static final String FIELD_DELTA     = "Delta (ms)";
    private static final String FIELD_FRAME_CRC = "Frame CRC";
    private static final String FIELD_CALC_CRC  = "Calc CRC";

    private static final List<String> EXTRA_FIELDS = Arrays.asList(
        FIELD_COUNT, FIELD_DELTA, FIELD_FRAME_CRC, FIELD_CALC_CRC
    );

    @Override
    public String getParserName() {
        return "Hoermann Gateway RS485";
    }

    @Override
    public String getSupportedFileType() {
        return "Hoermann Gateway RS485 bus log files";
    }

    @Override
    public List<String> getUserDefinedFields() {
        return EXTRA_FIELDS;
    }

    @Override
    protected void parseLine(String line) throws Exception {
        if (line == null) {
            // End of file – flush last entry
            if (entry != null) {
                addEntry(entry);
                entry = null;
            }
            return;
        }

        lineCount++;
        if (lineCount <= 2) {
            // Skip the first two lines (file header)
            return;
        }

        Matcher m = ENTRY_PATTERN.matcher(line);
        if (!m.matches()) {
            return;
        }

        // Flush previous entry
        if (entry != null) {
            addEntry(entry);
        }

        entry = createNewEntry();
        entry.setDate(m.group(1));
        entry.setEmitter(String.format("rs485.%s", m.group(2).trim()));  // Tx/Rx
        entry.setMessage(m.group(3).trim());  // frame bytes
        String crcStatus = m.group(6).trim();
        String level = "ok".equalsIgnoreCase(crcStatus) ? "INFO"
                     : "error".equalsIgnoreCase(crcStatus) ? "ERROR"
                     : crcStatus;
        entry.setLevel(level);

        HashMap<String, Object> fields = new HashMap<String, Object>(4);
        fields.put(FIELD_COUNT,     m.group(4).trim());
        fields.put(FIELD_DELTA,     m.group(5).trim());
        fields.put(FIELD_FRAME_CRC, m.group(7).trim());
        fields.put(FIELD_CALC_CRC,  m.group(8).trim());
        entry.setUserDefinedFields(fields);
    }

    @Override
    public Date getRelativeEntryDate(ParsedEntry pEntry) {
        return null; // no relative timestamp in this format
    }

    @Override
    public Date getAbsoluteEntryDate(ParsedEntry pEntry) throws Exception {
        if (dateFormat == null) {
            dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", getLocale());
        }
        synchronized (DATE_FORMATTER_MUTEX) {
            return dateFormat.parse(pEntry.getDate());
        }
    }
}
