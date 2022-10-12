using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


namespace ActiveRagdoll
{
    public class GripModule : BaseModule
    {
        public JointMotionsConfig defaultMotionsConfig;

        private Gripper _gripperRight, _gripperLeft;

        //References
        [SerializeField] private Transform _ragdollTransform; //Used for ThrowDirection

        //Layers
        private LayerMask _playerLayer; //Layer that's used when we're NOT trying to grip
        public LayerMask PlayerLayer { get { return _playerLayer; } set { _playerLayer = value; } }

        private LayerMask _gripLayer; //Layer that's used when we ARE trying to grip
        public LayerMask GripLayer { get { return _gripLayer; } set { _gripLayer = value; } }

        private bool _isGrabbing;
        public bool IsGrabbing { get { return _isGrabbing; } set { _isGrabbing = value; } }



        private void Start()
        {
            var leftHand = _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftHand).gameObject;
            var rightHand = _activeRagdoll.GetRagdollBone(HumanBodyBones.RightHand).gameObject;

            (_gripperLeft = leftHand.AddComponent<Gripper>()).GripMod = this;
            _gripperLeft.RagdollTransform = _ragdollTransform;
            _gripperLeft.PlayerID = _ragdollTransform.gameObject.GetComponent<PlayerController>().PlayerID;

            (_gripperRight = rightHand.AddComponent<Gripper>()).GripMod = this;
            _gripperRight.RagdollTransform = _ragdollTransform;
            _gripperRight.PlayerID = _ragdollTransform.gameObject.GetComponent<PlayerController>().PlayerID;
            _gripperLeft.gameObject.layer = _gripLayer; //Set Layer so "Gripper" only can grab "PlayerGrabColliders" and "PhysicalObjects"
            _gripperRight.gameObject.layer = _gripLayer; //Set Layer so "Gripper" only can grab "PlayerGrabColliders" and "PhysicalObjects"
        } //Start()

        public void GrippEnable()
        {
            _gripperLeft.enabled = true;
            _gripperRight.enabled = true;

            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightUpperArm).GetComponent<Rigidbody>().freezeRotation = true;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightLowerArm).GetComponent<Rigidbody>().freezeRotation = true;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightHand).GetComponent<Rigidbody>().freezeRotation = true;

            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftUpperArm).GetComponent<Rigidbody>().freezeRotation = true;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftLowerArm).GetComponent<Rigidbody>().freezeRotation = true;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftHand).GetComponent<Rigidbody>().freezeRotation = true;

            _activeRagdoll.GetRagdollBone(HumanBodyBones.Hips).GetComponent<Rigidbody>().freezeRotation = true;

            foreach (ConfigurableJoint cj in _activeRagdoll.GetComponentsInChildren<ConfigurableJoint>())
            {
                if (cj.GetComponent<Rigidbody>().freezeRotation == true)
                    continue;

                cj.angularXMotion = ConfigurableJointMotion.Free;
                cj.angularYMotion = ConfigurableJointMotion.Free;
                cj.angularZMotion = ConfigurableJointMotion.Free;
            }

        }

        public void GrippDisable()
        {

            _gripperLeft.enabled = false;
            _gripperRight.enabled = false;

            foreach (ConfigurableJoint cj in _activeRagdoll.GetComponentsInChildren<ConfigurableJoint>())
            {
                if (cj.GetComponent<Rigidbody>().freezeRotation == true)
                    continue;

                cj.angularXMotion = ConfigurableJointMotion.Limited;
                cj.angularYMotion = ConfigurableJointMotion.Limited;
                cj.angularZMotion = ConfigurableJointMotion.Limited;
            }

            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightUpperArm).GetComponent<Rigidbody>().freezeRotation = false;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightLowerArm).GetComponent<Rigidbody>().freezeRotation = false;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.RightHand).GetComponent<Rigidbody>().freezeRotation = false;

            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftUpperArm).GetComponent<Rigidbody>().freezeRotation = false;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftLowerArm).GetComponent<Rigidbody>().freezeRotation = false;
            _activeRagdoll.GetRagdollBone(HumanBodyBones.LeftHand).GetComponent<Rigidbody>().freezeRotation = false;

            _activeRagdoll.GetRagdollBone(HumanBodyBones.Hips).GetComponent<Rigidbody>().freezeRotation = false;
        }

    } //GripModule class

} // ActiveRagdoll namespace
