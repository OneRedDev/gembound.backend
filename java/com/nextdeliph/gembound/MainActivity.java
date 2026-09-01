package com.nextdeliph.gembound;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Color;
import android.graphics.Typeface;
import android.view.Gravity;
import android.widget.Button;
import android.widget.GridLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("gembound");
    }

    private TextView screen;

    private native void nativeStart();
    private native String nativeCommand(String command);
    private native String nativeScreen();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(Color.BLACK);

        screen = new TextView(this);
        screen.setTextColor(Color.WHITE);
        screen.setTextSize(18);
        screen.setTypeface(Typeface.MONOSPACE);
        screen.setGravity(Gravity.TOP | Gravity.START);
        screen.setPadding(20, 20, 20, 20);

        ScrollView scroll = new ScrollView(this);
        scroll.addView(screen);

        root.addView(
            scroll,
            new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                0,
                1
            )
        );

        GridLayout buttons = new GridLayout(this);
        buttons.setColumnCount(4);

        addButton(buttons, "W", "w");
        addButton(buttons, "A", "a");
        addButton(buttons, "S", "s");
        addButton(buttons, "D", "d");

        addButton(buttons, "F", "f");
        addButton(buttons, "1", "1");
        addButton(buttons, "2", "2");
        addButton(buttons, "SHOP", "p");

        addButton(buttons, "QUIT", "q");

        root.addView(buttons);

        setContentView(root);

        // Diagnostic mode:
        // Do NOT start the C++ game yet.
        screen.setText(
            "GEMBOUND ANDROID\n\n" +
            "Java layer loaded successfully.\n\n" +
            "C++ startup is temporarily disabled."
        );
    }

    private void addButton(
        GridLayout grid,
        String label,
        String command
    ) {
        Button button = new Button(this);
        button.setText(label);

        button.setOnClickListener(v -> {
            screen.append("\nButton: " + label);
        });

        GridLayout.LayoutParams params =
            new GridLayout.LayoutParams();

        params.width = 0;
        params.height = 65;
        params.columnSpec =
            GridLayout.spec(GridLayout.UNDEFINED, 1f);

        grid.addView(button, params);
    }
}
