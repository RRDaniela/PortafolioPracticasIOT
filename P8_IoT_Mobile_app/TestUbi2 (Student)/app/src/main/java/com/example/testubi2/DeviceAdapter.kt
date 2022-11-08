package com.example.testubi2

import android.content.Intent
import android.content.res.Resources
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.browser.customtabs.CustomTabsClient.getPackageName
import androidx.core.content.ContextCompat.startActivity
import androidx.recyclerview.widget.RecyclerView
import com.squareup.picasso.Picasso


class DeviceAdapter (private val dataList: ArrayList<DeviceClass>): RecyclerView.Adapter<DeviceAdapter.DeviceViewHolder>(){
    lateinit var deviceImage : ImageView

    class DeviceViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val deviceName : TextView = itemView.findViewById(R.id.deviceName)
        val deviceImage : ImageView = itemView.findViewById(R.id.deviceImg)

    }


    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): DeviceViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.devices_row, parent, false)
        deviceImage = view.findViewById(R.id.deviceImg)

        return DeviceViewHolder(view)
    }

    override fun onBindViewHolder(holder: DeviceViewHolder, position: Int) {
        holder.deviceName.text = dataList[position].name
        Picasso.get().load(dataList[position].photo).into(deviceImage)

        holder.deviceImage.setOnClickListener {
            val context=holder.deviceImage.context
            val intent = Intent( context, MainActivity::class.java)
            context.startActivity(intent)
        }

    }

    override fun getItemCount(): Int {
        if (dataList.size > 0){
            return dataList.size
        }else{
            return 0
        }
    }
}