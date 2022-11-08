package com.example.testubi2

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.ImageButton
import com.google.firebase.auth.FirebaseAuth

import android.widget.EditText
import android.widget.Toast


class ForgotPassword : AppCompatActivity() {
    private lateinit var  correo: EditText
    private lateinit var  forgotPass_btn: Button
    private lateinit var  auth: FirebaseAuth

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_forgot_pasword)

        correo = findViewById(R.id.forgotpass_mail)
        forgotPass_btn = findViewById(R.id.forgotpass_sendBtn)
        auth = FirebaseAuth.getInstance();

        val forgotpass_Backbtn : ImageButton = findViewById(R.id.forgotpass_backbtn)
        forgotpass_Backbtn.setOnClickListener(){
            val intent = Intent(this, LoginActivity::class.java)
            startActivity(intent)
        }

        forgotPass_btn.setOnClickListener(){
            try{
                auth.sendPasswordResetEmail(correo.text.toString())
                    .addOnCompleteListener { task ->
                        if (task.isSuccessful) {
                            Toast.makeText(this, "Revise su correo para restablacer su contraseña!", Toast.LENGTH_LONG).show()
                            val intent = Intent(this, LoginActivity::class.java)
                            startActivity(intent)
                        }else{
                            Toast.makeText(this, task.exception?.message, Toast.LENGTH_SHORT).show()
                        }
                    }
            }catch (e:IllegalArgumentException){
                Toast.makeText(this, "Ingresar correo!", Toast.LENGTH_SHORT).show()
            }
        }
    }
}