package com.jarvis.facedemo

import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.jarvis.facedemo.R

class VideoClipAdapter(
    private val clipIds: List<Int>,
    private val onItemClick: (Int) -> Unit
) : RecyclerView.Adapter<VideoClipAdapter.ViewHolder>() {

    private var selectedPosition = RecyclerView.NO_POSITION

    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val tvClipId: TextView = view.findViewById(R.id.tv_clip_id)
    }

    override fun onCreateViewHolder(viewGroup: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(viewGroup.context)
            .inflate(R.layout.item_video_clip, viewGroup, false)
        return ViewHolder(view)
    }

    override fun onBindViewHolder(viewHolder: ViewHolder, position: Int) {
        val clipId = clipIds[position]
        viewHolder.tvClipId.text = clipId.toString()
        
        // Update UI based on selection state
        if (position == selectedPosition) {
            viewHolder.tvClipId.setBackgroundColor(Color.parseColor("#FF6200EE")) // Purple 500
            viewHolder.tvClipId.setTextColor(Color.WHITE)
        } else {
            viewHolder.tvClipId.setBackgroundColor(Color.parseColor("#88000000"))
            viewHolder.tvClipId.setTextColor(Color.WHITE)
        }

        viewHolder.itemView.setOnClickListener {
            val currentPosition = viewHolder.adapterPosition
            if (currentPosition != RecyclerView.NO_POSITION) {
                notifyItemChanged(selectedPosition)
                selectedPosition = currentPosition
                notifyItemChanged(selectedPosition)
                onItemClick(clipId)
            }
        }
    }
    
    fun setSelected(position: Int) {
        val previousSelected = selectedPosition
        selectedPosition = position
        notifyItemChanged(previousSelected)
        notifyItemChanged(selectedPosition)
    }

    override fun getItemCount() = clipIds.size
}
