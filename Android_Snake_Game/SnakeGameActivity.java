import android.os.Bundle;
import android.view.View;
import stanford.androidlib.SimpleActivity;

public class SnakeGameActivity extends SimpleActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_snake_game);

        SnakeGameView game = find(R.id.snakegameview);
        game.startGame();
    }

    public void leftRotateClick(View view) {
        SnakeGameView game = findById(R.id.snakegameview);
        game.turnLeft();
    }

    public void rightRotateClick(View view) {
        SnakeGameView game = findById(R.id.snakegameview);
        game.turnRight();
    }
}
