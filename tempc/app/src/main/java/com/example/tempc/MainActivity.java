package com.example.tempc;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.example.tempc.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'tempc' library on application startup.
    static {
        System.loadLibrary("tempc");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        TextView tv2 = binding.textView;
        tv2.setText(process());
        //binding.sampleText.setText(process());
    }
    public native String process();
    public String processInJava(){
        return "Processed in Java accessed by C++";
    }

    /**
     * A native method that is implemented by the 'tempc' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}