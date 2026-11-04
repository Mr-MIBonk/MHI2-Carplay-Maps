package org.omonob.cpmap.ui;

/** The public information module is experimental in every supported locale. */
public final class InformationMenuMessages {
    private static final String[] LOCALES={"en","zh_CN","zh_TW","ja","ko","de","fr","es","it","pt","pt_BR","nl","ru","cs","sk","pl","sv","da","nb","fi","hu","ro","tr","el","uk","hr","sl","ar","he","th","id"};
    private static final String[] EXPERIMENT={
        "experimental","试验","試驗","試験","실험","experimentell","expérimental",
        "experimental","sperimentale","experimental","experimental","experimenteel",
        "эксперимент","experimentální","experimentálne","eksperymentalne",
        "experimentell","eksperimentel","eksperimentell","kokeellinen","kísérleti",
        "experimental","deneysel","πειραματικό","експеримент","eksperimentalno",
        "poskusno","تجريبي","ניסיוני","ทดลอง","eksperimental"
    };
    private static final String[] COMPACT={
        "CP info","CarPlay 信息","CarPlay 資訊","CP 情報","CP 정보","CP-Info","Infos CP",
        "Info CP","Info CP","Info CP","Info CP","CP-info","Данные CP","Info CP","Info CP","Info CP",
        "CP-info","CP-info","CP-info","CP-tiedot","CP-adatok","Info CP","CP bilgisi","Πληροφ. CP",
        "Дані CP","CP info","CP info","معلومات CP","מידע CP","ข้อมูล CP","Info CP"
    };
    private InformationMenuMessages() { }
    public static String marker(String language) {
        String code=UiMessages.normalize(language);
        for(int i=0;i<LOCALES.length;i++)if(LOCALES[i].equals(code))return EXPERIMENT[i];
        return EXPERIMENT[0];
    }
    public static String label(String language) {
        return new StringBuffer(UiMessages.get(language,UiMessages.INFORMATION))
            .append(" (").append(marker(language)).append(')').toString();
    }
    public static String compactLabel(String language) {
        String code=UiMessages.normalize(language);int selected=0;
        for(int i=0;i<LOCALES.length;i++)if(LOCALES[i].equals(code)){selected=i;break;}
        return new StringBuffer(COMPACT[selected]).append(" (").append(EXPERIMENT[selected]).append(')').toString();
    }
}
