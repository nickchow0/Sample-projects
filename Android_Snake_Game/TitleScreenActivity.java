import android.os.Bundle;
import android.view.View;
import stanford.androidlib.SimpleActivity;

public class TitleScreenActivity extends SimpleActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_title_screen);
    }

    public void playClick(View view) {
        startActivity(SnakeGameActivity.class);
    }

    public void exitClick(View view) {
        finish();
    }
}
