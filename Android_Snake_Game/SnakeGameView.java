import android.content.*;
import android.graphics.*;
import android.media.Image;
import android.util.AttributeSet;
import android.widget.Toast;
import java.util.*;
import stanford.androidlib.graphics.*;
import stanford.androidlib.util.*;
import android.content.Intent;
import android.os.Bundle;
import android.view.*;
import android.widget.*;
import stanford.androidlib.SimpleActivity;
import java.util.Scanner;

public class SnakeGameView extends GCanvas {

    public SnakeGameView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    private int dx = 5;
    private int dy = 0;
    private GSprite food;
    private GSprite snake;
    private ArrayList<GSprite> body;
    private GLabel score;
    private int scoreCount=0;

    @Override
    public void init() {
        score = new GLabel();
        score.setText("Score: " + scoreCount);
        score.setFontSize(40);
        add(score, getWidth() / 2, score.getHeight());
        snake = new GSprite();
        Bitmap head = BitmapFactory.decodeResource(
                getResources(), R.drawable.snakehead);
        Bitmap newSize = Bitmap.createScaledBitmap(head, 40, 40, true);
        snake.setBitmap(newSize);
        snake.setSize(newSize.getWidth(), newSize.getHeight());
        snake.setCollisionMargin(5);
        snake.setVelocityX(dx);
        snake.setVelocityY(dy);
        add(snake, getWidth() / 2, getHeight() / 2);
        food = new GSprite();
        Bitmap foodImage = BitmapFactory.decodeResource(
                getResources(), R.drawable.food);
        Bitmap newFoodSize = Bitmap.createScaledBitmap(foodImage, 40, 40, true);
        food.setBitmap(newFoodSize);
        food.setSize(newFoodSize.getWidth(), newFoodSize.getHeight());
        food.setCollisionMargin(5);
        body = new ArrayList<GSprite>();
        body.add(snake);
        RandomGenerator rgen = RandomGenerator.getInstance();
        int xLocation = rgen.nextInt(0,getWidth()/2);
        int yLocation = rgen.nextInt(0,getHeight()/2);
        add(food,xLocation, yLocation );
        animate(15);
    }

    private void shift(GPoint location, int count){
        if(count>=body.size()) return;
        GSprite ahead = body.get(count);
        GPoint point = ahead.getLocation();
        int xAhead = (int)ahead.getX();
        int yAhead = (int)location.getY();
        int xLocation = (int)location.getX();
        int yLocation = (int)location.getY();
            if(dx==5) xLocation-=ahead.getWidth();
            if(dx==-5) xLocation+=ahead.getWidth();
            if(dy==5) yLocation-=ahead.getHeight();
            if(dy==-5)yLocation+=ahead.getHeight();
        ahead.setLocation(xLocation, yLocation);
        location = point;
        shift(location, count + 1);
    }

    @Override
    public void onAnimateTick() {
        super.onAnimateTick();
        if (!body.isEmpty()) {
            if (body.size() > 1) {

                GPoint point = snake.getLocation();
                shift(point, 1);
            }
        }
        RandomGenerator rgen = RandomGenerator.getInstance();
        int xLocation = rgen.nextInt(0, getWidth() / 2);
        int yLocation = rgen.nextInt(0, getHeight() / 2);
        if (food.collidesWith(snake)) {
            food.setLocation(xLocation, yLocation);
            for (int index = 0; index < body.size(); index++) {
                GSprite bodyPart = body.get(index);
                if (food.collidesWith(snake) || food.collidesWith(bodyPart)) {
                    xLocation = rgen.nextInt(0, getWidth() / 2);
                    yLocation = rgen.nextInt(0, getHeight() / 2);
                    food.setLocation(xLocation, yLocation);
                }
            }
            addSegment();
            scoreCount++;
            score.setText("Score: " + scoreCount);
        }
    }

    private void addSegment(){
        Bitmap snakeBody = BitmapFactory.decodeResource(
            getResources(), R.drawable.snakebody);
        Bitmap Resized = Bitmap.createScaledBitmap(snakeBody, 40, 40,true);
        GSprite end = new GSprite();
        end.setBitmap(Resized);
        end.setSize(Resized.getWidth(), Resized.getHeight());
        GSprite ahead = body.get(body.size()-1);
        int xLocation = (int)ahead.getX();
        int yLocation = (int)ahead.getY();
        int xHead = (int)snake.getX();
        int yHead = (int)snake.getY();
        if(yHead==yLocation){
            if(dx==5) xLocation-=ahead.getWidth();
            if(dx==-5) xLocation+=ahead.getWidth();
        } else if(xHead==xLocation){
            if(dy==5) yLocation-=ahead.getHeight();
            if(dy==-5)yLocation+=ahead.getHeight();
        } else if(yHead>yLocation) {
            yLocation-=ahead.getHeight();
        }
        else if(yHead<yLocation){
            yLocation+=ahead.getHeight();
        }
        add(end, xLocation,yLocation);
        body.add(end);
    }
    public void startGame() {
    }

    public void turnLeft() {
         if(dy==0){
             if(dx==5) {
                 dx=0;
                 dy=-5;
             }
             else{
                 dx=0;
                 dy=5;
             }
         }else{
             if(dy==5){
                 dx=5;
                 dy=0;
             }else{
                 dx=-5;
                 dy= 0;
             }
         }
        snake.setVelocityX(dx);
        snake.setVelocityY(dy);
    }

    public void turnRight() {
        int dyCopy = dy;
        int dxCopy = dx;
        if(dy==0){
            if(dx==5) {
                dx=0;
                dy=5;
            }
            else{
                dx=0;
                dy=-5;
            }
        }else{
            if(dy==5){
                dx=-5;
                dy=0;
            }else{
                dx=5;
                dy = 0;
            }
        }
        snake.setVelocityX(dx);
        snake.setVelocityY(dy);
    }
}
